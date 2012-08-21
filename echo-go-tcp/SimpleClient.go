package main
import "net"

func main() {
  conn, _ := net.Dial("tcp", "localhost:1234")
  defer conn.Close()
  conn.Write([]byte("Hello world!"))
}
