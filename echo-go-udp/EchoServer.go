package main

import (
  "os"
  "net"
)

func handleClient(pconn *net.PacketConn, addr net.Addr,  msg []byte, n int) {
  println("message received from " + addr.String())
  (*pconn).WriteTo(msg, addr)
}

func main() {
  msg := make([]byte, 2048)

  pconn, err := net.ListenPacket("udp", "localhost:4242")

  if err != nil {
    println("Error listening" + err.Error())
    os.Exit(-1)
  }
  defer pconn.Close()

  for {
    n, addr, err := pconn.ReadFrom(msg)
    if err != nil {
      println(err)
      break
    }
    go handleClient(&pconn, addr, msg, n)
  }
}
