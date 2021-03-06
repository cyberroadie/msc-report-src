package main

import (
	"flag"
	"log"
	"net"
	"os"
)

type Settings struct {
	Address *string
	Message *string
	Verbose *bool
	Sctp    *bool
}

var (
	settings = Settings{
		Address: flag.String("a", "localhost:4242", "address to listen on"),
		Message: flag.String("m", "recv", "message to send"),
		Sctp:    flag.Bool("s", false, "Use SCTP"),
		Verbose: flag.Bool("v", false, "extra logging"),
	}
)

func echoServer(Address *string, Message *string) {
	var c net.PacketConn
	var err error

	c, err = net.ListenPacket("sctp", *settings.Address)

	if err != nil {
		log.Printf("Error listening: %v", err)
		os.Exit(-1)
	}
	defer c.Close()
	log.Printf("Listening on %s", *settings.Address)

	msg := make([]byte, 2048)
	for {
		_, _, err := c.ReadFrom(msg)
		if err != nil {
			log.Printf("Error: %v ", err)
			break
		}
	}

}

func main() {
	flag.Parse()
	echoServer(settings.Address, settings.Message)
}
