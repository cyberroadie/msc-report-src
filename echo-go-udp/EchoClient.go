package main

import (
  "net"
  "os"
)

func main() {
  msg := make([]byte, 2048)

  pconn, err := net.ListenPacket("udp", "localhost:4141")

  if err != nil {
    println("Error listening " + err.Error())
    os.Exit(-1)
  }
  defer pconn.Close()

  addr, err := net.ResolveUDPAddr("udp", "localhost:4242")
  if err != nil {
    println(err)
    os.Exit(-1)
  }

  for i := 0; i < 10; i++ {
   _, err := pconn.WriteTo([]byte("hello"), addr)
   if err != nil {
      println(err)
      break
    }
    _, _, err = pconn.ReadFrom(msg)
   if err != nil {
      println(err)
      break
    }
    println(string(msg))
  }
}
