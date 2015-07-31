sudo ip link set enp0s8 up
sudo pkt-gen -i enp0s8 -f tx -n 1024 -l 60 -d 74.125.224.72:80
sudo ip link set enp0s8 down