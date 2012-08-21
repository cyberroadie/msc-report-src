package main

import (
	"fmt"
	"image"
	"image/draw"
	"image/png"
	"image/color"
	"log"
	"os"
	"math/rand"
)

func main() {
	m := image.NewRGBA(image.Rect(0, 0, 400, 400))
    blue := color.RGBA{0, 0, 0, 0}
    draw.Draw(m, m.Bounds(), &image.Uniform{blue}, image.ZP, draw.Src)
	drawFern(m, 400, 400, 100000)
	f, err := os.OpenFile("fern.png", os.O_CREATE | os.O_WRONLY, 0666)
	if(err != nil) {
		log.Fatal(err)
	}
	if err = png.Encode(f, m); err != nil {
		log.Fatal(err)
	}
	fmt.Println("Done")
}

func drawFern(m *image.RGBA, x float32, y float32, steps int) {
	if steps != 0 {
		x, y = transform(x, y)
		drawPoint(m, x, y)
		drawFern(m, x, y, steps - 1)
	}
}

func drawPoint(m *image.RGBA, x float32, y float32) {
	b := m.Bounds()
	height := float32(b.Max.Y)
	width := float32(b.Max.X )
	scale := float32(height / 11)
    y = (height - 25) - (scale * y)
    x = (width / 2) + (scale * x)
	m.Set(int(x), int(y), color.RGBA{0, 255, 0, 255})
}

func transform(x float32, y float32) (float32, float32) {
	rnd := rand.Intn(101)
	switch {
		case rnd == 1: x, y = transformPoint(x,y, 0.0, 0.0, 0.0, 0.16, 0.0)
		case rnd <= 7: x, y = transformPoint(x,y, 0.2, -0.26, 0.23, 0.22, 0.0)
		case rnd <= 14: x, y = transformPoint(x,y, -0.15, 0.28, 0.26, 0.24, 0.44)
		case rnd <= 100: x, y = transformPoint(x,y, 0.85, 0.04, -0.04, 0.85, 1.6)
	}
	return x, y
}

func transformPoint(x, y, a, b, c ,d , s float32) (float32, float32) {
	return ((a * x) + (b * y)), ((c * x) + (d * y) + s)
}
