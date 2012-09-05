package main
import (
  "net"
  "strconv"
)

func main() {
  conn, _ := net.ListenPacket("sctp", "localhost:4242")
  var sconn = conn.(*net.SCTPConn)
  defer sconn.Close()
  message := make([]byte, 5)
  for {
    _, _, stream, _ := sconn.ReadFromSCTP(message)
    println("stream " + strconv.Itoa(int(stream)) + ": " + string(message))
  }
}
