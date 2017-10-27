var Canvas = require('..')
  , clock = require('./clock')

// Change to point at your framebuffer device
var fbdev = '/dev/fb0'

// TODO - resolution must match framebuffer resolution, or bad things will 
// happen (scrambled output, crashes)
var canvas = Canvas.createCanvas(480, 272, 'fb:' + fbdev)

// TODO -- 32 bit ARGB framebuffer is assumed, test/handle other cases
var ctx = canvas.getContext('2d', {pixelFormat: 'RGBA'})

function draw(ctx) {
	// Wobble the clock reference frame
	ctx.translate(Math.random() * 2 - 1, Math.random() * 2 - 1)
	ctx.rotate((Math.random() * 1 - 0.5) * Math.PI / 180)

	// Draw the clock
	clock(ctx)
}

var lastloop
function event_loop() {
	var now = (new Date).getTime()
	draw(ctx)

	// Draw fps display in upper left
	if (lastloop != undefined) {
		ctx.save()
		ctx.resetTransform()
		ctx.font = 'normal 20px Arial, sans'
		ctx.clearRect(400, 5, 200, 25)
		ctx.fillStyle = '#0f0'
		ctx.fillText('fps: ' + Math.floor(1000 / (now - lastloop)), 400, 25)
		ctx.restore()
	}

	canvas.toBuffer('fbblit')
	lastloop = now
	setTimeout(event_loop, 1)
}

// Start the renderer
event_loop()
