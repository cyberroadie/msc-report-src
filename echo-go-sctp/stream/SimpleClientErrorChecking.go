package main

import "net"

func main() {
	addr, _ := net.ResolveSCTPAddr("sctp", "localhost:4242")
	conn, err := net.DialSCTP("sctp", nil, addr)
  if err != nil {
    print(err)
    return
  }
	defer conn.Close()
	message := []byte("paard")
  _, er2 := conn.WriteToSCTP(message, addr, 1)
  if er2 != nil {
    print(er2.Error())
    return
  }
	conn.WriteToSCTP(message, addr, 2)
}
