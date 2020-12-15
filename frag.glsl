#version 440 core
out vec4 FragColor;
uniform int maxIterations = 4096;
uniform double RE_BEGIN = -2.5, IM_BEGIN = -1.0, WIDTH = 3.75, HEIGHT = 2.5;
uniform ivec2 windowsize;

struct Complex { double x, y; };

Complex power(const Complex z, const int p)
{
	Complex res = z;
	for (int i = 0; i < p-1; ++i) { double prevx = res.x; res.x = res.x*z.x - res.y*z.y;  res.y = prevx * z.y + z.x * res.y; }
	return res;
}

Complex add(const Complex z1, const Complex z2) { Complex sum; sum.x = z1.x + z2.x; sum.y = z1.y + z2.y; return sum; }

void main()
{
	Complex z, c; z.x = 0; z.y = 0;
	c.x = RE_BEGIN + WIDTH * double(gl_FragCoord.x) / double(windowsize.x); c.y = IM_BEGIN + HEIGHT * double(gl_FragCoord.y) / double(windowsize.y);
	for (int n = 0; n < maxIterations; ++n)
	{
		double zxprev = z.x; z.x = c.x + z.x*z.x - z.y*z.y; z.y = c.y + double(2.0) * zxprev * z.y; 
		//z = add(power(z, 2),c);
		
		//z.y = -z.y;
		//z = add(power(z, 5),c);
		//double zxprev = z.x; z.x = c.x + z.x*z.x - z.y*z.y; z.y = c.y - double(2.0) * zxprev * z.y; 
		
		//if (z.x*z.x + z.y*z.y >= 4) { FragColor = vec4(0.0, double(n)/double(maxIterations) * 0.8 + 0.2, 0.0, 1.0); return; }
		//if (z.x*z.x + z.y*z.y >= 4) { double x = double(n)/double(maxIterations) * 0.6 + 0.4; FragColor = vec4(0.0, x, 0.0, 1.0); return; }
		if (z.x*z.x + z.y*z.y >= 4) { FragColor = vec4(0.5 * sin(0.1 * n) + 0.5, 0.5 * sin(0.1 * n + 2.904) + 0.5, 0.5 * sin(0.1 * n + 4.188) + 0.5, 1.0); return; }
	}

	// burning Ship
	/*for (int n = 0; n < maxIterations; ++n)
	{
		double zxprev = z.x; z.x = c.x + z.x*z.x - z.y*z.y; z.y = c.y - abs(double(2.0) * zxprev * z.y); 
		//z.x = abs(z.x); z.y = -abs(z.y); z = add(power(z, 2), c);// will also work
		//if (z.x*z.x + z.y*z.y >= 4) { FragColor = vec4(double(n)/double(maxIterations) * 0.8 + 0.2,double(n)/double(maxIterations) * 0.8 + 0.2, double(n)/double(maxIterations) * 0.8 + 0.2, 1.0); return; }
		if (z.x*z.x + z.y*z.y >= 4) { FragColor = vec4(0.5 * sin(0.1 * n) + 0.5, 0.5 * sin(0.1 * n + 2.904) + 0.5, 0.5 * sin(0.1 * n + 4.188) + 0.5, 1.0); return; }
	}*/
	FragColor = vec4(0.0, 0.0, 0.0, 1.0);
}