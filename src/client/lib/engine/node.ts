import { Entity, type Metadata } from './entity';
import { Point } from './point';
import type { RenderEngine } from './renderEngine';

export class DummyNode extends Entity {
	public update(metadata: Metadata): void {
		if (metadata.mouse?.position) {
			this.position = metadata.mouse.position.clone();
		}
	}

	public render(renderEngine: RenderEngine): void {
		renderEngine.fillCircle(this.position, 37.5, 'white');
		renderEngine.circle(this.position, 37.5);
	}

	public selectedBy(point: Point): boolean {
		return point.distanceTo(this.position) <= 37.5;
	}

	public serialize(): never {
		throw new Error('Attempting to serialize a placeholder entity');
	}
}

export class Node extends Entity {
	constructor(public label: string) {
		super();
	}

	public update(metadata: Metadata): void {
		if (metadata.selected && metadata.mouse && metadata.mouse.down) {
			this.position = this.position.add(metadata.mouse.delta);
		}
	}

	public render(renderEngine: RenderEngine, metadata: Metadata): void {
		renderEngine.fillCircle(this.position, 20, 'white');

		if (metadata.selected) {
			renderEngine.fillCircle(this.position, 20, 'rgba(200, 200, 255, 0.5)');
		}

		renderEngine.circle(this.position, 20);

		renderEngine.text(this.position, this.label);
	}

	public selectedBy(point: Point): boolean {
		return point.distanceTo(this.position) <= 20;
	}
}

