package main

import (
	"flag"
	"log"
	"net"
	"os"
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

	addr, err := net.ResolveSCTPAddr("sctp", *settings.Address)
	if err != nil {
		println(err)
		os.Exit(-1)
	}

	conn, err := net.DialSCTP("sctp", nil, addr)

	if err != nil {
		println("Error listening " + err.Error())
		os.Exit(-1)
	}
	defer conn.Close()

	log.Printf("Dialing to %s", *settings.Address)

	var message = *settings.Message
	bmessage := []byte(message)

	for i := 0; i < *settings.Count; i++ {
		_, err := conn.WriteTo(bmessage, addr)
		if err != nil {
			log.Printf("WriteTo error: %v", err)
			break
		}
	}
}
