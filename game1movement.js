// game1movement.js
function myTest(a, b) {
	print('Vector a: x = ' + a.x() + ', y = ' + a.y() + '\n');
	print('Vector b: x = ' + b.x() + ', y = ' + b.y() + '\n');
	print('Result a * b = ' + a.dot(b) + '\n');
	print('Distance b/n a & b = ' + a.distance(b) + '\n');
	return 'Success!';
}
