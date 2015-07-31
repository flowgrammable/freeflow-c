// Copyright (c) 2014-2015 Flowgrammable.org
// All rights reserved

/* System headers */
#include <pthread.h>
#include <signal.h>

/* NADK headers */
/* NADK DEBUG defines must exist before processing NADK's headers */
#define NADK_LOGLIB_DISABLE 1
#define NADK_DEBUG 1
#include <nadk.h>
#include <nadk_types.h>
#include <nadk_common.h>
#include <nadk_dev.h>
#include <nadk_mbuf.h>
#include <nadk_ether.h>
#include <nadk_ethdev.h>
#include <nadk_time.h>
#include <ncs_ip.h>
#include <nadk_byteorder.h>
#include <nadk_lock.h>

#include "port_nadk.h"
#include "packet.h"
#include "port.h"


/* NADK Defines: */
#define NADK_FP_RX_BUDGET 16  /* Number frames received at a time */
#define NADK_FP_MIN_REQ_VQ 1
//#define NADK_FP_VQ_INDEX 0
#define NADK_FP_DATA_MEM_SIZE (4*1024*1024) /* 4 MB */
#define NADK_FP_BUF_NUM 512  /* Number of buffers released in	QBMAN */
#define NADK_FP_BUF_SIZE 2048  /* Ceil_2[Eth_Hdr(14) + data(1500) + FCS(4)] */
#define NADK_FP_MAX_ETH_DEV 16
#define NADK_FP_MAX_CONC_DEV 8
//#define NADK_FP_MIN_REQ_TCS 1  /* Minimum number of TC's required */
#define NADK_FP_TC_INDEX 0  /* TC index to use for distribution */
#define NADK_FP_SLEEP_MS 1  /* Sleep time */
#define NADK_FP_MAX_IO_THREADS 32


/* Internal NADK helper Structs: */
/*
 * Each thread is represented by a "worker" struct. It will be exited
 * once a CTRL+C is received.
 */
struct fp_nadk_worker
{
  pthread_t id;
  uint8_t cpu;
  uint32_t io_context;
  uint8_t dev_idx;
  uint8_t vq_idx;
};

struct fp_nadk_resources
{
  uint32_t eth_dev_cnt;
  uint32_t conc_dev_cnt;
  uint32_t io_context_cnt;
};


/* NADK Globals: */
typedef enum {NADK_UNINIT, NADK_INIT, NADK_ERROR} fp_nadk_state_t;
static fp_nadk_state_t fp_nadk_state = NADK_UNINIT;
static struct fp_nadk_resources nadk_res = {0};
static struct nadk_dev* net_dev[NADK_FP_MAX_ETH_DEV];
static struct nadk_dev* conc_dev[NADK_FP_MAX_CONC_DEV];
static struct fp_nadk_worker worker_attr[NADK_FP_MAX_IO_THREADS];
//struct fp_nadk_worker thread_attr[NADK_REFLECTOR_MAX_NUM_THREAD];

/* RX Ring Queues for NADK devices, allocated on port open */
static struct fp_ring* rx_rings[NADK_FP_MAX_ETH_DEV];
static struct fp_ring* tx_rings[NADK_FP_MAX_ETH_DEV];


/* The virtual table for all NADK devices. */
static struct fp_device_vtbl nadk_vtbl = {
  .recv  = fp_nadk_recv,
  .send  = fp_nadk_send,
  .drop  = fp_nadk_drop,
  .close = fp_nadk_close
};


/* Internal function to initialize NADK framekwork from a given Resource
 *   Container (DPRC).
 * - This function should only be called once, and resources should be
 *   cleaned up on any exit (error or planned).
 * TODO: proper error handling
 * TODO: ensure proper destruction on any sig-int or exit
 */
struct fp_nadk_resources*
fp_nadk_init(const char* dprc)
{
  /* Setup and configure NADK */
  struct nadk_init_cfg cfg = {0};

  /* Hard-coded NADK configs */
  cfg.vfio_container = dprc;
  cfg.data_mem_size = NADK_FP_DATA_MEM_SIZE;  // 4 MB
  cfg.log_level = NADK_LOG_DEBUG;
  cfg.flags = NADK_SYSTEM_INFO | NADK_SHOW_VERSION;
// NADK_PREFETCH_MODE |


  /* Initialize the NADK framekwork before any NADK methods are called! */
  printf("fp_nadk_init() about to initialize NADK framework\n");
  if (nadk_init(&cfg) != NADK_SUCCESS) {
    printf("NADK Error: nadk_init failed\n");
    goto fail_nadk_init;
  }
  printf("NADK successfully initialized\n");


  /* Determine number of I/O Contexts in Resource Container:
   * - We require at least 1 I/O context.
   * - Each thread that directly interacts with NADK send/recv functions
   *   requires thier own I/O context which must be afined to their ThreadID.
   */
  nadk_res.io_context_cnt = nadk_get_io_context_count();
  if (nadk_res.io_context_cnt == 0) {

    NADK_ERR(APP1, "NADK Error: no io context to run in");
    goto fail_nadk_init;
  }
  NADK_INFO(APP1, "Total [%d] IO SPACE Contexts available",
            nadk_res.io_context_cnt);


  /* Determine number of network devices in Resource Container:
   * - nadk_device_list should already contain the discovered deviced during
   *   nadk_init.
   * - For each device, the application has to initialize the RX/TX
   *   virtual queues that will be used.
   */
  NADK_INFO(APP1, "Determine number of network devices in DPRC: %s", dprc);
  struct nadk_dev* dev = NULL;
  TAILQ_FOREACH(dev, &device_list, next) {
    switch (dev->dev_type) {
    case NADK_NIC:
      NADK_NOTE(APP1, "DPNI => %d - %s discovered in DPRC",
                nadk_res.eth_dev_cnt, dev->dev_string);
      if (nadk_res.eth_dev_cnt < NADK_FP_MAX_ETH_DEV) {
        net_dev[nadk_res.eth_dev_cnt++] = dev;
      }
      else {
        NADK_INFO(APP1, "DPNI => %d - %s ignored due to %d DPNI device limit",
                  nadk_res.eth_dev_cnt, dev->dev_string,
                  NADK_FP_MAX_ETH_DEV);
      }
      break;
    // NOTE: we record the concentrator devices, but aren't using them
    case NADK_CONC:
      NADK_NOTE(APP1, "DPCON => %d - %s discovered in DPRC",
                nadk_res.conc_dev_cnt, dev->dev_string);
      if (nadk_res.conc_dev_cnt < NADK_FP_MAX_CONC_DEV) {
        conc_dev[nadk_res.conc_dev_cnt++] = dev;
      }
      else {
        NADK_INFO(APP1, "DPCON => %d - %s ignored due to %d DPCON device limit",
                  nadk_res.conc_dev_cnt, dev->dev_string,
                  NADK_FP_MAX_CONC_DEV);
      }
      break;
    default:
      NADK_INFO(APP1, "Unknown device in DPRC %x-%s",
                dev->dev_type, dev->dev_string);
    }
  }
  if (nadk_res.eth_dev_cnt == 0) {
    NADK_ERR(APP1, "No NADK_NIC devices discovered in DPRC");
    goto fail_nadk_init;
  }


  /* Affine thread which will send/recv to an I/O Context:
   * - This thread will use an I/O context EXCLUSIVELY.
   * - Multiple I/O Contexts can be requested for multi-threaded send/recv.
   */
  NADK_NOTE(APP1, "Affine current thread to an I/O Context");
  struct fp_nadk_worker base_attr;  // this is temporary...
  base_attr.cpu = 0;  // not used in this case
  base_attr.io_context = 1;  // use only one io context (single threaded)
  base_attr.id = pthread_self();  // not sure if this is needed
  worker_attr[0] = base_attr;// temporary
  worker_attr[1] = base_attr;// temporary

  NADK_INFO(APP1, "Affining io_context(%d) with thread (%lu)",
        worker_attr[0].io_context, worker_attr[0].id);
  int ret = nadk_thread_affine_io_context(worker_attr[0].io_context);
  if (ret != NADK_SUCCESS) {
    /* Error indicates that NADK is not able to allocate I/O Context */
    NADK_ERR(APP1, "Failure to affine threads with IO context");
    goto fail_nadk_init;
  }


  /* Initialize all Network Interfaces (DPNI):
   * - TODO: This is how the example does it. may need to move to port open...
   */
  /////nadk_reflector_configure_all_devices()
  /* Initialize Buffer Pool/s:
   * - In this case, all DPNI will share a single buffer pool.
   * - All DPNIs may also have a dedicated buffer poll.
   */
  /////config_buffer_pools()
  NADK_NOTE(APP1, "Initialize buffer pool");
  struct nadk_bp_list_cfg bp_cfg;
  memset(&bp_cfg, 0, sizeof(bp_cfg));
  bp_cfg.num_buf_pools = 1;
  bp_cfg.buf_pool[0].num = NADK_FP_BUF_NUM;
  bp_cfg.buf_pool[0].size = NADK_FP_BUF_SIZE;
//  bp_cfg.buf_pool[0].user_priv_size = 8;
  struct nadk_bp_list* bp_list = nadk_mbuf_pool_list_init(&bp_cfg);
  if (bp_list == NULL) {
    NADK_ERR(APP1, "Failure to initialize buffer pool/s");
    goto fail_nadk_init;
  }


  /* Attach Buffer Pool/s to DPNIs:
   * - Each NADK_NIC must be attached to a buffer pool before it can start
   *   sending and receiving packets.
   * - TODO: This is how the example does it. may need to move to port open...
   */
  for (int i = 0; i < nadk_res.eth_dev_cnt; i++) {
    dev = net_dev[i];

    NADK_NOTE(APP1, "port => %d/%d - %s being created",
              i + 1, nadk_res.eth_dev_cnt, dev->dev_string);

    /* Attach configured buffer pool list to this device */
    NADK_INFO(APP1, "Attach buffer pool to DPNI: %d", i);
    ret = nadk_eth_attach_bp_list(dev, bp_list);
    if (ret != NADK_SUCCESS) {
      NADK_ERR(APP1, "Failure to attach buffers to the Ethernet device");
      printf("Failure to attach buffers to the Ethernet device");
      goto fail_nadk_init;
    }

    /* Get available RX & TX Virtual Queues (VQs) for this device */
    NADK_INFO(APP1, "Get available RX & TX VQs for DPNI: %d", i);
    uint32_t max_rx_vq = nadk_dev_get_max_rx_vq(dev);
    uint32_t max_tx_vq = nadk_dev_get_max_tx_vq(dev);
    if (max_rx_vq < NADK_FP_MIN_REQ_VQ || max_tx_vq < NADK_FP_MIN_REQ_VQ) {
      NADK_ERR(APP1, "Not enough VQs to run:\nRX: %d/%d\tTX: %d/%d",
               max_rx_vq, NADK_FP_MIN_REQ_VQ, max_tx_vq, NADK_FP_MIN_REQ_VQ);
      goto fail_nadk_init;
    }

    /* Configure Virtual Queues (VQs) for this device */
    NADK_NOTE(APP1, "Configure VQs for DPNI: %d", i);
    struct queues_config* q_config = nadk_eth_get_queues_config(dev);
    if (q_config == NULL || q_config->num_tcs < NADK_FP_MIN_REQ_VQ) {
      NADK_ERR(APP1, "Number of actual RX VQs less than expected: %d/%d",
               q_config->num_tcs, NADK_FP_MIN_REQ_VQ);
      goto fail_nadk_init;
    }


    /* Set up Flow Distribution:
     * - Determines how packets are destributed to VQs.
     * - Required to make use of more than one VQ.
     */
//    uint32_t req_dist_set = NADK_FDIST_IP_DA;
    uint32_t rx_vq = NADK_FP_MIN_REQ_VQ;  // TODO: only single-threaded..
//    uint32_t tx_vq = NADK_FP_MIN_REQ_VQ;  // TODO: only single-threaded..
//    ret = nadk_eth_setup_flow_distribution(dev, req_dist_set,
//                                           NADK_FP_TC_INDEX, rx_vq);
//    if (ret != NADK_SUCCESS) {
//      NADK_ERR(APP1, "Failed to configure RX Flow Distribution\n");
//      printf("Failed to configure RX Flow Distribution\n");
//      goto fail_nadk_init;
//    }
    /* TODO: set the per thread device distribution rule */
    /* no distrobution enabled with single thread: */
//    worker_attr[i] = base_attr;


    /* Setup RX Virtual Queues (VQs) for this device.
     * - TODO: this only uses VQ 0, as this we are only single-threaded.
     */
    NADK_NOTE(APP1, "Setup RX VQs for DPNI: %d", i);
    //for (int j = 0; j < max_rx_vq; j++) {
      ret = nadk_dev_setup_rx_vq(dev, 0, NULL);
      if (ret != NADK_SUCCESS) {
        NADK_ERR(APP1, "Fail to setup RX VQ[%d] for device[%d]", 0, i);
        NADK_ERR(APP1, "Failed to configure RX VQs %d: dev=%d, budget=%d/%d",
                 0, i, rx_vq, max_rx_vq);
        goto fail_nadk_init;
      }
    //}
    worker_attr[i].dev_idx = i;
    worker_attr[i].vq_idx = 0;


    /* Start Network Device: */
    NADK_NOTE(APP1, "Starting DPNI: %d", i);
    ret = nadk_dev_start(dev);
    if (ret != NADK_SUCCESS) {
      NADK_ERR(APP1, "Failed to start network device: %d", i);
      goto fail_nadk_init;
    }


    /* Setup TX Virtual Queues (VQs) for this device:
     * - Due to a hardware issue, the TX VQs must be setup last as a workaround.
     *   TX VQs should ideally be setup before device start.
     */
    NADK_NOTE(APP1, "Setup TX VQs for DPNI: %d", i);
    ret = nadk_dev_setup_tx_vq(dev, max_tx_vq, NADKBUF_TX_NO_ACTION);
    if (ret != NADK_SUCCESS) {
      NADK_ERR(APP1, "Fail to setup %d TX VQs for device[%d]", max_tx_vq, i);
      goto fail_nadk_init;
    }


    /* Print information about the (now active) port */
    NADK_NOTE(APP1, "Attempting to set eth MAC address for DPNI: %d", i);
    struct nadk_eth_addr addr;
    if (nadk_eth_get_mac_addr(dev, &addr) == NADK_SUCCESS) {
      NADK_NOTE(APP1, "Dev: %d; Port Id: %s", i, dev->dev_string);
      NADK_NOTE(APP1, "Eth Addr: %.2x:%.2x:%.2x:%.2x:%.2x:%.2x",
                addr.addr[0], addr.addr[1], addr.addr[2],
                addr.addr[3], addr.addr[4], addr.addr[5]);
    }
  }
  /* On sucessful NADK initialization and device discovery, mark initialized */
  fp_nadk_state = NADK_INIT;
  atexit(fp_nadk_cleanup);
  return &nadk_res;


  /* Failure during NADK Initialization.  Clean up all resources! */
  fail_nadk_init:
  printf("Failed fp_nadk_init() for DPRC: %s\n", dprc);
  // A lot more stuff needs to go here...
  fp_nadk_state = NADK_ERROR;
  return NULL;
}


/* Attempt to cleanup all NADK resources.  Currently only supports
 * cleaning up nadk if sucessfully initialized (NADK_INIT status).
 */
void
fp_nadk_cleanup()
{
  if (fp_nadk_state == NADK_UNINIT) {
    printf("NADK already cleaned up\n");
    return;
  }

  NADK_INFO(APP1, "Begin NADK cleanup");
  /* this thread already has an I/O context (required for cleanup) */

  /*Clear buffer library*/
  nadk_mbuf_finish();

  /*Gracefull shutdown to all the Ethernet devices*/
  for (int i = 0; i < nadk_res.eth_dev_cnt; i++) {
    int ret = nadk_dev_stop(net_dev[i]);
    if (ret != NADK_SUCCESS) {
      NADK_ERR(APP1, "Failed to stop device: %d", i);
    }

    ret = nadk_dev_shutdown(net_dev[i]);
    if (ret != NADK_SUCCESS)
      NADK_ERR(APP1, "Failed to shutdown device: %d", i);
  }
  /*Gracefull shutdown to all the Concentrator devices*/
//  for (i = 0; app_opts.conc_mode && i < nadk_res.conc_dev_cnt; i++)
//    nadk_dev_stop(conc_dev[i]);

  /* Do cleanup and exit */
  nadk_cleanup();
  printf("Finished NADK cleanup\n");
  fp_nadk_state = NADK_UNINIT;
}


/* Attempt to open NADK port */
struct fp_device*
fp_nadk_open(short p)
{
  //static struct fp_nadk_resources* rc = NULL;

  /* Initialize nadk runtime if haven't already for this flowpath process:
   * - TODO: this may just need to return an error as flowmgr should have already
   *   queried flowpath for available ports.
   */
  if (fp_nadk_state == NADK_UNINIT) {
    fp_nadk_init("dprc.6");
  }
  if (fp_nadk_state == NADK_ERROR) {
    fprintf(stderr, "Error: NADK framework not properly initialized\n");
    return NULL;
  }


  // finish setting up individual ports here...
  if (p < 0 && p >= nadk_res.eth_dev_cnt) {
    fprintf(stderr, "Error: NADK RC does not contain port %d\n", p);
    return NULL;
  }

  rx_rings[p] = fp_ring_new(2*NADK_FP_RX_BUDGET);
  if (rx_rings[p] == NULL) {
    fprintf(stderr, "Error: Failed to allocate an RX Ring for Port %d\n", p);
  }
  tx_rings[p] = fp_ring_new(2*NADK_FP_RX_BUDGET);
  if (tx_rings[p] == NULL) {
    fprintf(stderr, "Error: Failed to allocate an TX Ring for Port %d\n", p);
  }

  /* Build the device. */
  struct fp_nadk_device* dev = fp_allocate(struct fp_nadk_device);
  dev->base.vtbl = &nadk_vtbl;
  dev->handle = net_dev[p];  // TODO: replace with initialized port
  dev->rx_ring = rx_rings[p];
  dev->tx_ring = tx_rings[p];
  dev->dprc_id = p;
  NADK_INFO(APP1, "NADK opened port %d", p);

  return (struct fp_device*)dev;
}


/* Close and destroy the NADK port. The object pointed to by port
   is no longer valid after this operation. */
void
fp_nadk_close(struct fp_device* device)
{
  struct fp_nadk_device* dev = (struct fp_nadk_device*)device;
//  nm_close(dev->handle);
  fp_ring_delete(dev->rx_ring);
  fp_ring_delete(dev->tx_ring);
  fp_deallocate(dev);
}


/* Performs a batch recv request and stores into local queue for
 * fp_nadk_recv().  This is more efficient than simply requesting a
 * single packet from the NADK framekwork.
 */
int
fp_nadk_recv_batch(struct fp_device* device)
{
  struct fp_nadk_device* dev = (struct fp_nadk_device*)device;
  struct nadk_dev* ndev = dev->handle;
  struct nadk_mbuf* pkt_buf[NADK_FP_RX_BUDGET];
  struct fp_packet* rx_batch[NADK_FP_RX_BUDGET];

  int ret = nadk_receive(ndev, ndev->rx_vq[0], NADK_FP_RX_BUDGET, pkt_buf);
  if (unlikely(ret <= 0))
    return 0; /* nothing waiting */
  if (unlikely(ret > NADK_FP_RX_BUDGET))
    NADK_ERR(APP1, "Somehow recieved more packets than alloted budget...");

  /* Allocate a flowpath packet. */
  for (int i = 0; i < ret; i++) {
    rx_batch[i] = fp_packet_create(pkt_buf[i]->data, pkt_buf[i]->length,
                                   pkt_buf[i]->timestamp,
                                   pkt_buf[i], FP_BUF_NADK);
  }

  return fp_ring_push_n(dev->rx_ring, (void**)rx_batch, ret);
}


/* Read from a NADK device, creating a new packet. */
struct fp_packet*
fp_nadk_recv_single(struct fp_device* device)
{
  struct fp_nadk_device* dev = (struct fp_nadk_device*)device;
  struct nadk_dev* ndev = dev->handle;
  struct nadk_mbuf* pkt_buf[1];
  struct fp_packet* packet;

  int ret = nadk_receive(ndev, ndev->rx_vq[0], 1, pkt_buf);
  if (unlikely(ret <= 0))
    return NULL; /* nothing waiting */
  if (unlikely(ret > 1))
    NADK_ERR(APP1, "FATAL ERROR: somehow recieved more packets than alloted budget...");

  /* Allocate a flowpath packet. */
  packet = fp_packet_create(pkt_buf[0]->data, pkt_buf[0]->length,
                            pkt_buf[0]->timestamp,
                            pkt_buf[0], FP_BUF_NADK);
  return packet;
}


/* Pull from the RX Ring, returning a single packet. */
struct fp_packet*
fp_nadk_recv(struct fp_device* device)
{
  struct fp_nadk_device* dev = (struct fp_nadk_device*)device;

  int count = fp_ring_count(dev->rx_ring);
  if (count < NADK_FP_RX_BUDGET) {
    fp_nadk_recv_batch(device);
  }

  return fp_ring_pop(dev->rx_ring);
}


/* Performs a batch send request from local queue from
 * fp_nadk_send().  This is more efficient than simply sending a
 * single packet to the NADK framekwork.
 */
int
fp_nadk_send_batch(struct fp_device* device)
{
  struct fp_nadk_device* dev = (struct fp_nadk_device*)device;
  struct nadk_dev* ndev = dev->handle;
  struct nadk_mbuf* pkt_buf[NADK_FP_RX_BUDGET];
  struct fp_packet* rx_batch[NADK_FP_RX_BUDGET];

  int num = fp_ring_top_n(dev->tx_ring, (void**)rx_batch, NADK_FP_RX_BUDGET);
  for (int i = 0; i < num; i++) {
    if (likely(rx_batch[i]->buf_dev == FP_BUF_NADK)) {
      pkt_buf[i] = rx_batch[i]->buf_handle;
    }
    else {
      // TODO: assuming NADK_BUF_ALLOC
      // - if another interface's buf type is recieved i.e. FP_BUF_NETMAP, need a
      //   callback to release that SDK's buffer type...
      pkt_buf[i] = nadk_mbuf_alloc(ndev, rx_batch[i]->size);
      if (unlikely(pkt_buf[i] == NULL)) {
        NADK_WARN(APP1, "Failed to allocate mbuf on %d/%d", i, num);
        num = i;
        break;
      }
      int ret = nadk_mbuf_data_copy_in(pkt_buf[i], rx_batch[i]->data, 0,
                                       rx_batch[i]->size);
      if (unlikely(ret != NADK_SUCCESS)) {
        NADK_WARN(APP1, "Failed to copy data to mbuf on %d/%d", i, num);
        num = i;
        nadk_mbuf_free(pkt_buf[i]);
        break;
      }
      fp_deallocate(rx_batch[i]->data);  // release allocated data segment
      // TODO: Do I need to set any other mbuf members?  i.e. eth src/dst?
    }
  }
  if (unlikely(num == 0))
    return 0;

  int sent = nadk_send(ndev, ndev->tx_vq[0], num, pkt_buf);
  if (unlikely(sent != num)) {
    NADK_WARN(APP1, "Failed to send %d/%d mbuf on demand...", sent, num);
  }

  for (int i = 0; i < sent; i++) {
    fp_packet_delete(rx_batch[i]);
  }
  fp_ring_remove_n(dev->tx_ring, sent);
  return sent;
}


/* Write to a NADK device, consuming a packet */
int
fp_nadk_send_single(struct fp_device* device, struct fp_packet* pkt)
{
  struct fp_nadk_device* dev = (struct fp_nadk_device*)device;
  struct nadk_dev* ndev = dev->handle;
  struct nadk_mbuf* pkt_buf[1];

  if (likely(pkt->buf_dev == FP_BUF_NADK)) {
    pkt_buf[0] = pkt->buf_handle;
  }
  else {
    // TODO: assuming NADK_BUF_ALLOC
    // - if another interface's buf type is recieved i.e. FP_BUF_NETMAP, need a
    //   callback to release that SDK's buffer type...
    pkt_buf[0] = nadk_mbuf_alloc(ndev, pkt->size);
    if (unlikely(pkt_buf[0] == NULL)) {
      NADK_WARN(APP1, "Failed to allocate mbuf...");
      goto abort_on_mbuf_alloc;
    }
    int ret = nadk_mbuf_data_copy_in(pkt_buf[0], pkt->data, 0, pkt->size);
    fp_deallocate(pkt->data);  // release allocated data segment
    if (unlikely(ret != NADK_SUCCESS)) {
      NADK_WARN(APP1, "Failed to copy data to mbuf...");
      goto abort_send;
    }
    // TODO: Do I need to set any other mbuf members?  i.e. eth src/dst?
  }

  int ret = nadk_send(ndev, ndev->tx_vq[0], 1, pkt_buf);
  if (unlikely(ret != 1)) {
    NADK_WARN(APP1, "Failed to send mbuf on demand...");
    goto abort_send;
  }

  fp_packet_delete(pkt);
  return ret = pkt->size;

  abort_send:
  nadk_mbuf_free(pkt->buf_handle);
  abort_on_mbuf_alloc:
  fp_packet_delete(pkt);
  return 0;
}


/* Push to the TX Ring, queueing a single packet. */
int
fp_nadk_send(struct fp_device* device, struct fp_packet* pkt)
{
  struct fp_nadk_device* dev = (struct fp_nadk_device*)device;

  int num = fp_ring_push(dev->tx_ring, (void*)pkt);
  // counting twice, unfortunately...
  if (fp_ring_count(dev->tx_ring) >= NADK_FP_RX_BUDGET) {
    // TODO: Implement a maximum wait time until send_batch()...
    fp_nadk_send_batch(device);
  }
  return num;
}


/* Drop a packet from a NADK interface. */
void
fp_nadk_drop(struct fp_device* device, struct fp_packet* pkt)
{
  fp_deallocate(pkt->data);
  fp_packet_delete(pkt);
}


/* NADK loopback test (bypasses flowpath's send/recv functions).
 * - All packets recieved on device will be reflected back.
 * - Used to test max PPS through interface.
 */
int
fp_nadk_loopback(struct fp_device* device)
{
  struct fp_nadk_device* dev = (struct fp_nadk_device*)device;
  struct nadk_dev* ndev = dev->handle;

#define BATCH 16
  struct nadk_mbuf* pkt_buf[BATCH];

  int received = nadk_receive(ndev, ndev->rx_vq[0], BATCH, pkt_buf);
  if (unlikely(received <= 0))
    return 0; /* nothing waiting */
  if (unlikely(received > BATCH))
    NADK_ERR(APP1, "FATAL ERROR: somehow recieved more packets than alloted budget...");

  int sent = nadk_send(ndev, ndev->tx_vq[0], received, pkt_buf);
  if (unlikely(sent != received)) {
    NADK_WARN(APP1, "Could only send %d/%d packets...", sent, received);
    // Cleanup packets that weren't sent:
    for (int i = sent; i < received; i++) {
      nadk_mbuf_free(pkt_buf[i]);
    }
  }

  return sent;
}


/* NADK loopback test (bypasses flowpath's send/recv functions).
 * - All packets recieved on src will be reflected back on dst.
 * - Used to test max PPS through interface.
 */
int
fp_nadk_xloopback(struct fp_device* src, struct fp_device* dst)
{
  struct fp_nadk_device* src_dev = (struct fp_nadk_device*)src;
  struct fp_nadk_device* dst_dev = (struct fp_nadk_device*)dst;
  struct nadk_dev* src_ndev = src_dev->handle;
  struct nadk_dev* dst_ndev = dst_dev->handle;

#define BATCH 16
  struct nadk_mbuf* pkt_buf[BATCH];

  int received = nadk_receive(src_ndev, src_ndev->rx_vq[0], BATCH, pkt_buf);
  if (unlikely(received <= 0))
    return 0; /* nothing waiting */
  if (unlikely(received > BATCH))
    NADK_ERR(APP1, "FATAL ERROR: somehow recieved more packets than alloted budget...");

  int sent = nadk_send(dst_ndev, dst_ndev->tx_vq[0], received, pkt_buf);
  if (unlikely(sent != received)) {
    NADK_WARN(APP1, "Could only send %d/%d packets...", sent, received);
    // Cleanup packets that weren't sent:
    for (int i = sent; i < received; i++) {
      nadk_mbuf_free(pkt_buf[i]);
    }
  }

  return sent;
}

