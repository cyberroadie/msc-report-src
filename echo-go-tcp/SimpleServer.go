package main
import "net"

func main() {
  listen, err := net.Listen("tcp", "localhost:1234")
  if err != nil {
    return
  }
  buffer := make([]byte, 1024)
  for {
    conn, err := listen.Accept()
    if err != nil {
      continue
    }
    conn.Read(buffer)
    println(string(buffer))
  }
}
