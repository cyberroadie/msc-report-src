package main

import (
        "flag"
        "net"
        "os"
        "io"
        "log"
        "encoding/binary"
)

type Settings struct {
        Address *string
        Message *string
        Verbose *bool
        Sctp    *bool
}

var (
        settings = Settings{
                Address: flag.String("a", "127.0.0.1:4242", "address to listen on"),
                Message: flag.String("m", "recv", "message to send"),
                Sctp: flag.Bool("s", false, "Use SCTP"),
                Verbose: flag.Bool("v", false, "extra logging"),
        }
)

func main() {
        flag.Parse()
        echoServer(settings.Address, settings.Message)
}