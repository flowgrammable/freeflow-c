// Copyright (c) 2014-2015 Flowgrammable.org
// All rights reserved

#ifndef FLOWGR_DEVICE_HPP
#define FLOWGR_DEVICE_HPP

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#include <arpa/inet.h>
#include <sys/socket.h>
#include <netdb.h>
#include <ifaddrs.h>
#include <linux/if_link.h>
#include <sys/types.h>

#include <list>

#include "freeflow/socket.hpp"

namespace flowmgr
{
// The flowmgr device module. This mostly wraps the C ifaddrs
// device interface to simplify its usage. Provides an iterator
// to access the list of network devices in the system.
//
// FIXME: Improve upon the device list class and condense the
// number of entries in the list. That is, recognize when a
// duplicate device interface is found (device name ==) and add
// the new information to the interface instance. This will require
// a re-working of how the device interface class operates.
namespace device
{

// A wrapper for the C ifaddrs device interface struct. Maintains
// a pointer to the raw data and provides access to its members.
//
// FIXME: Add a back end store to allow for multiple addresses (IPv4, IPv6, etc)
class interface
{
  using value_type = ifaddrs;
  using address_type = sockaddr;
  using info_type = struct rtnl_link_stats*;
public:
  // Constructors/Destructor.
  interface();
  interface(value_type&);
  ~interface();

  // Copy assignment operator.
  interface& operator=(interface&);  

  // Equality/Inequality comparators.
  inline bool operator==(interface&);
  inline bool operator!=(interface&);

  // Accessors.
  char const* name();
  unsigned int flags() const;
  std::list<address_type>::iterator address_begin();
  std::list<address_type>::iterator address_end();
  address_type const* netmask();
  address_type const* dest_address();
  address_type const* broad_address();
  info_type data();

  // Mutators.
  void add_address(address_type&);

private:
  value_type* data_;
  std::list<address_type> address_;
};


// Default constructor for a device interface.
interface::interface()
  : data_(nullptr)
{ }


// Destructor for a device interface.
interface::~interface()
{ 
  // FIXME: Implement this.
}


// Move constructor for a device interface.
interface::interface(value_type& other)
  : data_(std::move(&other))
{ }


// Copy assignment operator for a device inteface.
interface&
interface::operator=(interface& other)
{
  *this = other;
  return *this;
}


// Equality comparison for device interfaces.
inline bool
interface::operator==(interface& other)
{
  return name() == other.name();
}


// Inequality comparison for device interfaces.
inline bool
interface::operator!=(interface& other)
{
  return name() != other.name();
}


// Returns the name of the current interface.
char const*
interface::name()
{
  return data_->ifa_name;
}


// Returns the current flags set for the interface from SIOCGIFFLAGS.
unsigned int 
interface::flags() const
{
  return data_->ifa_flags;
}


// Returns an iterator at the beginning of the socket address(es)
// for the interface.
auto
interface::address_begin() -> std::list<address_type>::iterator
{
  return address_.begin();
}


// Returns an iterator at the end of the socket address(es) for
// the interface.
auto
interface::address_end() -> std::list<address_type>::iterator
{
  return address_.end();
}


// Returns the netmask for the interface.
auto
interface::netmask() -> address_type const*
{
  return (address_type const*)data_->ifa_netmask;
}

// Returns the broadcast address for the interface.
auto
interface::broad_address() -> address_type const*
{
  return (address_type const*)data_->ifa_ifu.ifu_broadaddr;
}


// Returns the P2P destination address for the interface.
auto
interface::dest_address() -> address_type const*
{
  return (address_type const*)data_->ifa_ifu.ifu_dstaddr;
}


// Returns a pointer to any additional data available for the interface.
auto
interface::data() -> info_type
{
  return (info_type)data_->ifa_data;
}


// Adds an additional socket address to the device interface.
void
interface::add_address(address_type& other)
{
  address_.push_front(other);
}

// The device interface list class. Creates a list of devices found
// in the system and provides an iterator to access them.
//
// FIXME: Change how the list constructor works; recognize when a duplicate
// device name is found and add that to the existing entry.
class list
{
  using value_type = interface; 
  using store_type = std::list<interface>;
public:

  // Constructor/Destructor.
  list();
  ~list();

  // Accessors.
  store_type::iterator begin();
  store_type::iterator end();

private:
  store_type data_;
};


// Default device list constructor. Initializes the device interface list.
list::list()
{ 
  ifaddrs* temp;
  getifaddrs(&temp);
  while (temp) {
    data_.push_front(interface(*temp));
    temp = temp->ifa_next;
  }
}


// Device list destructor.
list::~list()
{
  // FIXME: Implement this.
}


// Returns an iterator at the beginning of the device list.
auto
list::begin() -> store_type::iterator
{
  return data_.begin();
}


// Returns an iterator at the end of the device list.
auto
list::end() -> store_type::iterator
{
  return data_.end();
}


} // end namespace device


} // end namespace flowmgr


#endif
