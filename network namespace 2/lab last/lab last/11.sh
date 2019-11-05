#Connect two named network namespaces:
 #Change the bandwidth between them using the TBF qdisc



 sudo ip netns add red
 sudo ip netns add green

 sudo ip add link eth0 type veth peer name eth1

 sudo ip link set eth0 netns red up
 sudo ip link set eth1 netns red up

 sudo ip netns exec red ip link set lo up
 sudo ip netns exec green ip link set lo up

 sudo ip netns exec red ip link set eth0 up
 sudo ip netns exec green ip link set eth1 up

 sudo ip netns exec red ip address add 10.0.0.1/24 dev eth0
 sudo ip netns exec green ip address add 10.0.0.2/24 dev eth1


 
 sudo ip netns exec red tc qdisc add dev eth0 root tbf rate 1mbit burst 32kbit latency 400ms

 sudo ip netns exec red ping 10.0.0.2

  sudo ip netns delete red
  sudo ip netns delete green





