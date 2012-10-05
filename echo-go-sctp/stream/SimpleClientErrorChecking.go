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
	message := []byte("paarden")
  _, er2 := conn.WriteToSCTP(message, addr, 4)
  if er2 != nil {
    print(er2.Error())
    return
  }
	conn.WriteToSCTP(message, addr, 3)
}
