
# Same as scenario 2 except, in the Router, double the delay on the
# interface connected to the Sender, and add an AQM qdisc (RED, PIE, or CoDel)
# on the interface connected to the Receiver.

#cenario 1: Keep the delay and bandwidth on every interface the same.

sudo ip netns add sender-eth0--------eth1-router-eth2------------eth3-Receiver
sudo ip netns add router
sudo ip netns add Receiver

sudo ip link add eth0 type veth peer name eth1
sudo ip link add eth2 type veth peer name eth3

sudo ip link set eth0 netns sender
sudo ip link set eth1 netns router
sudo ip link set eth2 netns router
sudo ip link set eth3 netns Receiver

sudo ip netns exec sender ip link set lo up
sudo ip netns exec router ip link set lo up
sudo ip netns exec Receiver ip link set lo up

sudo ip netns exec sender ip link set eth0 up
sudo ip netns exec router ip link set eth1 up
sudo ip netns exec router ip link set eth2 up
sudo ip netns exec Receiver ip link set eth3 up

sudo ip netns exec sender ip address add 10.0.0.1/24 dev eth0
sudo ip netns exec router ip address add 10.0.0.2/24 dev eth1
sudo ip netns exec router ip address add 10.0.0.3/24 dev eth2
sudo ip netns exec Receiver ip address add 10.0.0.4/24 dev eth3


tc qdisc add dev eth0 root netem delay 200ms

	
tc qdisc add dev eth0 root netem delay 200ms
sudo ip netns exec router tc qdisc add dev eth2 root netem rate 1mbit burst 32kbit latency 400ms
sudo ip netns exec Receiver tc qdisc add dev eth3 root netem rate 1mbit burst 32kbit latency 400ms

#  sender-eth0--------eth1-router-eth2------------eth3-Receiver
