package main

import (
  "flag"
  "sync"
  "strconv"
	"image"
	"image/draw"
	"image/png"
	"image/color"
  "math/rand"
  "math"
  "os"
)

var height = flag.Int("h", 5000, "Image height")
var width = flag.Int("w", 5000, "Image width")

type Point struct {
  x, y float64
}

func main() {
  flag.Parse()
  var size = *height * *width

  var max = 1000000
  ci := make(chan int, max / 10)
  quit := make(chan bool)
  go collectData(ci, quit, size)
  wg := &sync.WaitGroup{}

  for i := 0; i < 2; i++ {
    var rng = max / 2
    var start = i * rng
    var end = i * rng + rng
    wg.Add(1)
    println("starting buddha loop " + strconv.Itoa(start) + " - " + strconv.Itoa(end))
    go buddhaLoop(start, end, ci, size, wg)
  }
  println("waiting")
  wg.Wait()
  println("closing channel")
  close(ci)

  <-quit
  println("done")

}

func buddhaLoop(start int, end int, ci chan int, size int, wg *sync.WaitGroup) {
  var points = make([]Point, size + 1)
  var ix, iy int
  var x, y float64

  for tt := start; tt < end; tt++ {
    for t := 0; t < 1000; t++ {
      x = 6 * rand.Float64() - 3
      y = 6 * rand.Float64() - 3

      if n, test := iterate(x, y, points); test == true {
        for i := 0; i<n; i++ {
          ix = int(0.3 * float64(*width) * (points[i].x + float64(0.5)) + float64(*width/2))
          iy = int(0.3 * float64(*height) * points[i].y + float64(*height/2))
          if ix >= 0 && iy >= 0 && ix < *width && iy < *height {
            var z = iy * *width + ix
            ci <- z
          }
        }
      }
    }
  }
  wg.Done()
  return
}

func collectData(ci chan int, quit chan bool, size int) {
  fractal := make([]uint64, size + 1)

  for {
    i, ok := <-ci
    if !ok {
      break
    }
    fractal[i] = fractal[i] + 1
  }

  println("calculation finished, writing image")
  writeImage(fractal, *width, *height)
  quit <- true

}


func iterate(x0 float64, y0 float64, p []Point) (int, bool) {
  var x, y, xnew, ynew float64;

  for i := 0; i < 200; i++ {
   xnew = x * x - y * y + x0;
   ynew = 2 * x * y + y0;

   p[i].x = xnew
   p[i].y = ynew

   if xnew*ynew + ynew*ynew > 10 {
     return i, true
   }
   x = xnew
   y = ynew
 }
 return 0, false
}

func writeImage(fractal []uint64, width int, height int) {
  var biggest, smallest uint64
  for _, v := range fractal {
    if biggest < v {
      biggest = v
    }
  }

  smallest = biggest
  for _, v := range fractal {
    if smallest > v {
      smallest = v
    }
  }
  println(smallest)
  println(biggest)

  var ramp float64
  m := image.NewRGBA(image.Rect(0, 0, width, height))
  background := color.RGBA{0, 0, 0, 0}
  draw.Draw(m, m.Bounds(), &image.Uniform{background}, image.ZP, draw.Src)
  for i, v := range fractal {
    ramp = float64(2)*(float64(v) - float64(smallest) / float64(biggest - smallest))
    if ramp > float64(1) {
      ramp = float64(1)
    }
    ramp = math.Pow(ramp, float64(0.5))
    //var rgb = int(ramp * float64(255))
    var x = i - (i / width) * width
    var y = (i - x) / width
    //m.Set(x, y, color.RGBA{uint8(rgb),uint8(rgb), uint8(rgb),255})
    m.Set(y, x, color.RGBA{0, 0, uint8(v), 255})
    //m.Set(x, y, color.RGBA{0, 0, 255, 255})
  }

	f, err := os.OpenFile("buddha.png", os.O_CREATE | os.O_WRONLY, 0666)
	if(err != nil) {
		println(err)
	}
	if err = png.Encode(f, m); err != nil {
		println(err)
	}
	println("Done")

}

