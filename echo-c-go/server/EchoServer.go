/*
 * User: cyberroadie
 * Date: 12/11/2011
 */
package main

import (
	"flag"
	"net"
	"os"
	"syslog"
	"io"
	"log"
	"encoding/binary"
)

type Settings struct {
	Address *string
	Message *string 
	Verbose *bool 
	Sctp bool 
}

var (
	settings = Settings {
	    Address: flag.String("a", "127.0.0.1:4242", "address to listen on"),
		Message: flag.String("m", "recv", "message to send"),
		Verbose: flag.Bool("v", false, "extra logging"),	
	}
)

func Test(err os.Error, mesg string) {
    if err!=nil {
        syslog, _ := syslog.New(syslog.LOG_ERR, "echo server")
        _, err = io.WriteString(syslog, mesg)
        os.Exit(-1);
    }
}

func HandleClient(conn net.Conn, message *string) {
	
	size, sizeBytes := uint32(0), make([]byte, 4)
	id, idBytes := uint32(0), make([]byte, 4)
	
	for {
		// Get message length
		if n, err := io.ReadFull(conn, sizeBytes); n == 0 && err == os.EOF {
			break
		} else if err != nil {
			log.Printf("read size: %s (after %d bytes)", err, n)
			break
		}
		// Get message id
		if n, err := io.ReadFull(conn, idBytes); n == 0 && err == os.EOF {
			break
		} else if err != nil {
			log.Printf("read id: %s (after %d bytes)", err, n)
			break
		}
		
		size = binary.BigEndian.Uint32(sizeBytes)
		data := make([]byte, size)
		if _, err := io.ReadFull(conn, data); err != nil {
			log.Printf("read data: %s", err)
			break
		}
		
		// Send message id back as ACK
		id = binary.BigEndian.Uint32(idBytes)
		//log.Printf("write data: %d", id)
		if _, err := conn.Write(idBytes); err != nil {
			log.Printf("write data: %d", id)
			log.Printf("write data: %s", err)
			break
		}
	}					
}

func EchoServer(Address *string, Message *string) {
	
	netlisten, err := net.Listen("tcp", *settings.Address)
	Test(err, "Listen() error")
    defer netlisten.Close()

	for {
	    c, err := netlisten.Accept();
    	Test(err, "Accept() error")

        go HandleClient(c, Message)
    	

	}	
}

func main() {
	flag.Parse()
	EchoServer(settings.Address, settings.Message)
}
