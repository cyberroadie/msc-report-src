package main

func receiveChan(ci chan int) {
  for {
    i := <-ci
    println(i)
  }
}

func main() {
  ci := make(chan int)
  go receiveChan(ci)

  for i := 0; i < 10; i++ {
    ci <- i
  }
}

