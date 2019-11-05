
#                 red1-eth0-------eth1-routet_namespace-eth3-----eth2-blue1


#  red blue namespaces
sudo ip netns add red1
sudo ip netns add blue1

sudo ip link add eth0 type veth peer name eth1
sudo ip link add eth2 type veth peer name eth3

sudo ip link set eth0 netns red1
sudo ip link set eth2 netns blue1

sudo ip netns exec red1 ip link set eth0 up
sudo ip netns exec blue1 ip link set eth2 up

sudo ip netns exec red1 ip address add 10.0.0.1/24 dev eth0
sudo ip netns exec blue1 ip address add 10.0.2.1/24 dev eth2


#       another namespace   router
sudo ip netns add router
sudo ip link set eth1 netns router
sudo ip link set eth3 netns router

sudo ip netns exec router ip link set eth1 up
sudo ip netns exec router ip link set eth3 up

sudo ip netns exec router ip address add 10.0.0.2/24 dev eth1
sudo ip netns exec router ip address add 10.0.2.2/24 dev eth3

sudo ip netns exec red1 ip link set lo up
sudo ip netns exec blue1 ip link set lo up
sudo ip netns exec router ip link set lo up

# default route
sudo ip netns exec red1 ip route add default via 10.0.0.2 dev eth0
sudo ip netns exec blue1 ip route add default via 10.0.2.2 dev eth2

sudo ip netns exec router sysctl -w net.ipv4.ip_forward=1

sudo ip netns exec blue1 ping 10.0.0.1

sudo ip netns delete red1
sudo ip netns delete blue1
sudo ip netns delete router
