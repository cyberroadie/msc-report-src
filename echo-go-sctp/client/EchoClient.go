package main

import (
  "flag"
  "net"
  "os"
  "log"
)

type Settings struct {
	Address   *string
	Message   *string
	Count     *int
	Instances *int
	Verbose   *bool
}

var (
	settings = Settings{
		Address:   flag.String("a", "localhost:4242", "address to sent messages to"),
		Message:   flag.String("m", "recv", "message to send"),
		Count:     flag.Int("c", 1, "number of messages"),
		Instances: flag.Int("n", 1, "number of threads"),
		Verbose:   flag.Bool("v", false, "extra logging"),
	}
)


func main() {
  flag.Parse()
  msg := make([]byte, 2048)

  conn , err := net.Dial("sctp", *settings.Address)
  var pconn = conn.(net.PacketConn)

  if err != nil {
    println("Error listening " + err.Error())
    os.Exit(-1)
  }
  defer pconn.Close()

  log.Printf("Dialing to %s", *settings.Address)

  addr, err := net.ResolveSCTPAddr("sctp", *settings.Address)
  if err != nil {
    println(err)
    os.Exit(-1)
  }

  var message = *settings.Message
  bmessage := []byte(message)

  for i := 0; i < 10; i++ {
   log.Printf("Sending message '%s'", message)
   _, err := pconn.WriteTo(bmessage, addr)
   if err != nil {
      log.Printf("WriteTo error: %v", err)
      break
    }
    _, _, err = pconn.ReadFrom(msg)
   if err != nil {
      println("ReadFrom error")
      print(err)
      break
    }
    println("Received: " + string(msg))
  }
}
