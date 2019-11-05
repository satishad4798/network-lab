sudo ip netns add red
sudo ip netns add green
sudo ip netns add blue

sudo ip link add eth0 type veth peer name eth1
sudo ip link add eth2 type veth peer name eth3

sudo ip link set eth0 netns red
sudo ip link set eth1 netns green
sudo ip link set eth2 netns green
sudo ip link set eth3 netns blue

sudo ip netns exec red ip link set lo up
sudo ip netns exec green ip link set lo up
sudo ip netns exec blue ip link set lo up

sudo ip netns exec red ip link set eth0 up
sudo ip netns exec green ip link set eth1 up
sudo ip netns exec green ip link set eth2 up
sudo ip netns exec blue ip link set eth3 up

sudo ip netns exec red ip address add 10.0.0.1/24 dev eth0
sudo ip netns exec green ip address add 10.0.0.2/24 dev eth1
sudo ip netns exec green ip address add 10.0.0.3/24 dev eth2
sudo ip netns exec blue ip address add 10.0.0.4/24 dev eth3

sudo ip netns exec red tc qdisc add dev eth0 root netem rate 1mbit burst 32kbit latency 400ms
sudo ip netns exec green tc qdisc add dev eth1 root netem rate 1mbit burst 32kbit latency 400ms
sudo ip netns exec green tc qdisc add dev eth2 root netem rate 1mbit burst 32kbit latency 400ms
sudo ip netns exec blue tc qdisc add dev eth3 root netem rate 1mbit burst 32kbit latency 400ms

#  red-eth0--------eth1-green-eth2------------eth3-blue