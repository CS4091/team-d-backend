import { Point } from './point';

export function calculateIntersection(c1: Point, c2: Point, r1: number, r2: number) {
	const R = c1.distanceTo(c2);
	const [x1, y1] = c1,
		[x2, y2] = c2;

	return [0.5, -0.5]
		.map((c) => c * Math.sqrt(2 * ((r1 ** 2 + r2 ** 2) / R ** 2) - (r1 ** 2 - r2 ** 2) ** 2 / R ** 4 - 1))
		.map((c) => new Point(y2 - y1, x1 - x2).times(c))
		.map((pt) => pt.add(new Point(x2 - x1, y2 - y1).times((r1 ** 2 - r2 ** 2) / (2 * R ** 2))))
		.map((pt) => pt.add(new Point(x1 + x2, y1 + y2).times(0.5)));
}

export function calculateLHIntersection(c1: Point, c2: Point, r1: number, r2: number) {
	const [, int] = calculateIntersection(c1, c2, r1, r2);

	return int;
}

export function calculateRHIntersection(c1: Point, c2: Point, r1: number, r2: number) {
	const [int] = calculateIntersection(c1, c2, r1, r2);

	return int;
}

export function movementDelta(movement: string): number {
	return movement === 'R' ? 1 : movement === 'L' ? -1 : 0;
}

