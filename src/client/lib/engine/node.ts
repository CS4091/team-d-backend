import { Entity, type Metadata } from './entity';
import { Point } from './point';
import type { RenderEngine } from './renderEngine';

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

	public selectedBy(point: Point, renderEngine: RenderEngine): boolean {
		return point.distanceTo(this.position) <= 20 / renderEngine.fov.scale;
	}
}

