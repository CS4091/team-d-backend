import { Entity, type Metadata } from './entity';
import type { Node } from './node';
import { Point } from './point';
import type { RenderEngine } from './renderEngine';
import { calculateIntersection, calculateLHIntersection, calculateRHIntersection } from './utils';

export enum CycleSide {
	LEFT,
	RIGHT
}

interface ArcData {
	start: Point;
	end: Point;
	center: Point;
	r: number;
	angle: number;
	tangent: Point;
	normal: Point;
	arcCenter: Point;
	endTangent: Point;
	endNormal: Point;
}

export class Edge extends Entity {
	constructor(public from: Node, public to: Node, public data: number, public color: string, public cycleState: CycleSide | null = null) {
		super();

		this.position = from.position.add(to.position).divide(2);
	}

	public update(): void {
		this.position = this.from.position.add(this.to.position).divide(2);
	}

	public render(renderEngine: RenderEngine, metadata: Metadata): void {
		const fromPos = this.from.position,
			toPos = this.to.position;

		if (this.cycleState === null) {
			if (this.from === this.to) {
				const center = this.from.position.add(new Point(0, 55));

				const [end, start] = calculateIntersection(this.from.position, center, 20, 25);
				const radVec = end.subtract(center);
				const endTangent = new Point(radVec.y, -radVec.x).scaleTo(1);
				const endNormal = new Point(endTangent.y, -endTangent.x).scaleTo(1);
				const r = 25;
				const sRad = start.subtract(center);
				const angle = 2 * Math.PI - Math.acos(sRad.dot(radVec) / (sRad.magnitude() * radVec.magnitude()));

				const arcCenter = center.add(new Point(0, 25));
				const normal = new Point(0, 1),
					tangent = new Point(1, 0);

				if (metadata.selected) {
					renderEngine.arc(start, center, r, angle, 'rgba(200, 200, 255, 0.5)', 6);
				}

				renderEngine.arc(start, center, r, angle, this.color);

				renderEngine.fillShape(
					[
						end,
						end.add(endTangent.invert().scaleTo(7.5)).add(endNormal.scaleTo(4)),
						end.add(endTangent.invert().scaleTo(4)),
						end.add(endTangent.invert().scaleTo(7.5)).add(endNormal.scaleTo(-4)),
						end
					],
					this.color
				);

				const conditionBasepoint = arcCenter.add(normal.scaleTo(5));

				renderEngine.text(conditionBasepoint, `${this._data()}`, { direction: tangent });
			} else {
				const dir = toPos.subtract(fromPos),
					perpRight = new Point(dir.y, -dir.x);
				const arrowTip = toPos.subtract(dir.scaleTo(20)),
					midpoint = fromPos.add(toPos).times(0.5);

				if (metadata.selected) {
					renderEngine.line(fromPos, toPos, 6, 'rgba(200, 200, 255, 0.5)');
				}

				renderEngine.line(fromPos, toPos, 1, this.color);
				renderEngine.fillShape(
					[
						arrowTip,
						arrowTip.add(dir.invert().scaleTo(7.5)).add(perpRight.scaleTo(4)),
						arrowTip.add(dir.invert().scaleTo(4)),
						arrowTip.add(dir.invert().scaleTo(7.5)).add(perpRight.scaleTo(-4)),
						arrowTip
					],
					this.color
				);

				const conditionBasepoint = midpoint.add(perpRight.scaleTo(-5));

				renderEngine.text(conditionBasepoint, `${this._data()}`, { direction: dir });
			}
		} else {
			const { start, center, angle, r, end: arrowTip, endTangent, normal, arcCenter, tangent, endNormal } = this._calculateArcData();

			if (metadata.selected) {
				renderEngine.arc(start, center, r, angle, 'rgba(200, 200, 255, 0.5)', 6);
			}

			renderEngine.arc(start, center, r, angle, this.color);

			renderEngine.fillShape(
				[
					arrowTip,
					arrowTip.add(endTangent.invert().scaleTo(7.5)).add(endNormal.scaleTo(4)),
					arrowTip.add(endTangent.invert().scaleTo(4)),
					arrowTip.add(endTangent.invert().scaleTo(7.5)).add(endNormal.scaleTo(-4)),
					arrowTip
				],
				this.color
			);

			const conditionBasepoint = arcCenter.add(normal.scaleTo(tangent.x < 0 ? -15 : -5));

			renderEngine.text(conditionBasepoint, `${this._data()}`, { direction: tangent.x < 0 ? tangent.invert() : tangent });
		}
	}

	public selectedBy(point: Point, renderEngine: RenderEngine): boolean {
		const fromPos = this.from.position,
			toPos = this.to.position;

		if (this.cycleState === null) {
			if (this.from === this.to) {
				const ctx = renderEngine.context;
				const center = this.from.position.add(new Point(0, 55));

				const [end, start] = calculateIntersection(this.from.position, center, 20, 25);
				const radVec = end.subtract(center);
				const r = 25;
				const sRad = start.subtract(center);
				const angle = 2 * Math.PI - Math.acos(sRad.dot(radVec) / (sRad.magnitude() * radVec.magnitude()));

				const [sx, sy] = renderEngine.spaceToCanvas(start);
				const [cx, cy] = renderEngine.spaceToCanvas(center);
				const delta = center.subtract(start);
				const startAngle = Math.atan2(delta.x, delta.y) + Math.PI / 2;

				const path = new Path2D();

				path.moveTo(sx, sy);
				path.arc(cx, cy, r, startAngle, startAngle + angle);

				const [px, py] = renderEngine.spaceToCanvas(point);

				ctx.lineWidth = 10;
				const selected = ctx.isPointInStroke(path, px, py);
				ctx.lineWidth = 1;

				return selected;
			} else {
				const length = fromPos.distanceTo(toPos);

				if (length === 0) {
					return fromPos.distanceTo(point) <= 5;
				}

				const projection = ((point.x - fromPos.x) * (toPos.x - fromPos.x) + (point.y - fromPos.y) * (toPos.y - fromPos.y)) / length ** 2;
				const clampedProjection = Math.max(0, Math.min(projection, 1));

				return point.distanceTo(fromPos.add(toPos.subtract(fromPos).times(clampedProjection))) <= 5;
			}
		} else {
			const { start, center, angle, r } = this._calculateArcData();

			const ctx = renderEngine.context;

			const path = new Path2D();
			const [sx, sy] = renderEngine.spaceToCanvas(start);
			const [cx, cy] = renderEngine.spaceToCanvas(center);
			const delta = center.subtract(start);
			const startAngle = Math.atan2(delta.x, delta.y) + Math.PI / 2;

			path.moveTo(sx, sy);
			path.arc(cx, cy, r, startAngle, startAngle + angle);

			const [px, py] = renderEngine.spaceToCanvas(point);

			ctx.lineWidth = 10;
			const selected = ctx.isPointInStroke(path, px, py);
			ctx.lineWidth = 1;

			return selected;
		}
	}

	private _calculateArcData(): ArcData {
		const fromPos = this.from.position,
			toPos = this.to.position;
		const x = toPos.distanceTo(fromPos) / 2,
			r = (x ** 2 + 625) / 50;

		const dir = toPos.subtract(fromPos),
			midpoint = fromPos.add(dir.scaleTo(fromPos.distanceTo(toPos) / 2)),
			perpRight = new Point(dir.y, -dir.x);

		const center = midpoint.add(perpRight.scaleTo(r - 25));

		const end = calculateLHIntersection(center, toPos, r, 20);
		const radVec = end.subtract(center);
		const endTangent = new Point(radVec.y, -radVec.x).scaleTo(1);
		const start = calculateRHIntersection(center, fromPos, r, 20);
		const endNormal = new Point(endTangent.y, -endTangent.x).scaleTo(1);

		const shortX = start.distanceTo(end) / 2;
		const angle = Math.asin(shortX / r) * 2;

		return {
			start,
			end,
			center,
			r,
			angle,
			endTangent,
			arcCenter: midpoint.add(perpRight.scaleTo(-25)),
			tangent: dir.scaleTo(1),
			normal: perpRight.scaleTo(1),
			endNormal
		};
	}

	private _data(): string {
		return Number.isInteger(this.data) ? `${this.data}` : `${this.data.toFixed(2)}`;
	}
}

