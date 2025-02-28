import { CycleSide, Edge } from './edge';
import type { Entity, MouseData } from './entity';
import { Node } from './node';
import { Point } from './point';
import { RenderEngine } from './renderEngine';

interface EngineEvents {
	entityClicked: (entity: Entity, metadata: { button: MouseButton; spacePos: Point; pagePos: Point }) => void;
	entityDblClicked: (entity: Entity) => void;
	click: (evt: MouseEvent) => void;
}

export enum MouseButton {
	LEFT,
	MIDDLE,
	RIGHT,
	BACK,
	FORWARD
}

interface GraphNode {
	id: string;
	x: number;
	y: number;
}

interface ConnectionEdge {
	from: number;
	to: number;
	data: {
		distance: number;
		duration: number;
	};
}

interface DemandEdge {
	from: number;
	to: number;
	data: number;
}

export class Engine {
	private readonly context: CanvasRenderingContext2D;
	private readonly layers: Entity[][] = [[], []];
	private readonly renderEngine: RenderEngine;

	private _selectedEntity: Entity | null = null;
	private _mousePos: Point | null = null;
	private _mouseDown = false;
	private _mouseDelta: Point | null = null;

	private _listeners: { [K in keyof EngineEvents]: EngineEvents[K][] };

	private mouseListener: (evt: MouseEvent) => void = (evt) => {
		if (this._mousePos) {
			this._mousePos = this.renderEngine.canvasToSpace(new Point(evt.offsetX, evt.offsetY));

			if (this._mouseDelta) {
				this._mouseDelta.x += evt.movementX;
				this._mouseDelta.y -= evt.movementY;
			}
		}
	};

	constructor(private readonly canvas: HTMLCanvasElement) {
		const ctx = canvas.getContext('2d');

		if (ctx) {
			this.context = ctx;
			this.renderEngine = new RenderEngine(ctx, canvas);

			this._listeners = { entityClicked: [], click: [], entityDblClicked: [] };

			canvas.addEventListener('mouseout', () => {
				this._mousePos = null;

				canvas.removeEventListener('mousemove', this.mouseListener);
			});

			canvas.addEventListener('mouseover', (evt) => {
				this._mousePos = new Point(evt.offsetX, evt.offsetY);

				canvas.addEventListener('mousemove', this.mouseListener);
			});

			canvas.addEventListener('mousedown', () => {
				this._mouseDown = true;
				this._mouseDelta = new Point();
			});

			canvas.addEventListener('mouseup', (evt: MouseEvent) => {
				this._mouseDown = false;
				this._mouseDelta = null;

				if (this._selectedEntity) {
					for (const listener of this._listeners.entityClicked) {
						listener(this._selectedEntity, {
							button: evt.button,
							spacePos: this._mousePos!,
							pagePos: this.renderEngine.spaceToCanvas(this._mousePos!).add(new Point(16, 52))
						});
					}
				} else {
					for (const listener of this._listeners.click) {
						listener(evt);
					}
				}
			});

			canvas.addEventListener('dblclick', () => {
				if (this._selectedEntity) {
					for (const listener of this._listeners.entityDblClicked) {
						listener(this._selectedEntity);
					}
				}
			});

			canvas.addEventListener('contextmenu', (evt: MouseEvent) => {
				if (this._selectedEntity) {
					evt.preventDefault();
				}
			});
		} else {
			throw new Error('Unable to get canvas context');
		}
	}

	public add(entity: Entity, layer: number): void {
		while (layer >= this.layers.length) {
			this.layers.push([]);
		}

		this.layers[layer].push(entity);
	}

	public remove(entity: Entity, layer?: number): void {
		if (layer === undefined) {
			for (const layer of this.layers) {
				if (layer.includes(entity)) {
					layer.splice(layer.indexOf(entity), 1);
				}
			}
		} else {
			if (!this.layers[layer]) {
				throw new Error(`Layer ${layer} does not exist!`);
			} else if (!this.layers[layer].includes(entity)) {
				throw new Error(`Layer ${layer} does not contain entity!`);
			} else {
				this.layers[layer].splice(this.layers[layer].indexOf(entity), 1);
			}
		}

		if (!(entity instanceof Edge)) {
			for (const layer of this.layers) {
				const toRemove = layer.filter((e) => e instanceof Edge && (e.from === entity || e.to === entity));
				toRemove.forEach((line) => layer.splice(layer.indexOf(line), 1));
			}
		} else {
			const doppel = this.findDoppel(entity);

			if (doppel) {
				doppel.cycleState = null;
			}
		}
	}

	public start(): void {
		this._tick();
	}

	public on<T extends keyof EngineEvents>(evt: T, listener: EngineEvents[T]): () => void {
		this._listeners[evt].push(listener);

		return () => {
			this._listeners[evt].splice(this._listeners[evt].indexOf(listener), 1);
		};
	}

	public connected(from: Node, to: Node): boolean {
		return this.layers.some((layer) => layer.some((entity) => entity instanceof Edge && entity.from === from && entity.to === to));
	}

	public findDoppel(edge: Edge): Edge | null {
		return (
			(this.layers
				.reduce((arr, layer) => [...arr, ...layer], [])
				.find((entity) => entity instanceof Edge && entity.from === edge.to && entity.to === edge.from) as Edge) || null
		);
	}

	public loadConnectivity(graph: { nodes: GraphNode[]; edges: ConnectionEdge[] }): void {
		this.layers.splice(0);

		const bboxBL = new Point(0, 0),
			bboxUR = new Point(0, 0);

		const nodes = graph.nodes.map(({ id, x, y }) => {
			const node = new Node(id);
			node.position = new Point(x, y);

			if (x < bboxBL.x) bboxBL.x = x;
			if (y < bboxBL.y) bboxBL.y = y;
			if (x > bboxUR.x) bboxUR.x = x;
			if (y > bboxUR.y) bboxUR.y = y;

			this.add(node, 1);

			return node;
		});

		const bboxDims = new Point(bboxUR.x - bboxBL.x, bboxUR.y - bboxBL.y),
			bboxCenter = new Point((bboxUR.x + bboxBL.x) / 2, (bboxUR.y + bboxBL.y) / 2);

		nodes.forEach((node) => (node.position = node.position.subtract(bboxCenter).scale(900 / bboxDims.x, 600 / bboxDims.y)));

		graph.edges.forEach(({ from, to, data }) => this.add(new Edge(nodes[from], nodes[to], data.duration, 'black'), 0));
	}

	public loadDemand(graph: { nodes: GraphNode[]; edges: DemandEdge[] }): void {
		const nodes = graph.nodes.map(({ id }) => {
			let node: Node;

			this.layers.forEach((layer) =>
				layer.forEach((n) => {
					if (n instanceof Node && n.label === id) {
						node = n;
					}
				})
			);

			return node;
		});

		graph.edges.forEach(({ from, to, data }) => this.add(new Edge(nodes[from], nodes[to], data, 'red'), 0));
	}

	private _tick(): void {
		requestAnimationFrame(() => this._tick());

		if (!this._mouseDown) {
			this._updateSelectedEntity();
		}

		if (this._selectedEntity) {
			this.canvas.style.cursor = 'pointer';
		} else {
			this.canvas.style.cursor = 'unset';
		}

		const relationshipLinks: [Node, Node, Edge][] = [];
		this.layers.forEach((layer) => {
			layer.forEach((entity) => {
				if (entity instanceof Edge) {
					let relationship: [Node, Node, Edge];
					if ((relationship = relationshipLinks.find(([from, to]) => entity.from === to && entity.to === from)!)) {
						relationship[2].cycleState = CycleSide.LEFT;
						entity.cycleState = CycleSide.RIGHT;

						relationshipLinks.splice(
							relationshipLinks.findIndex(([, , t]) => t === relationship[2]),
							1
						);
					} else if (entity.from !== entity.to) {
						relationshipLinks.push([entity.from, entity.to, entity]);
					}
				}
			});
		});

		this.layers.forEach((layer) => {
			layer.forEach((entity) => {
				if (entity instanceof Edge && entity.cycleState !== null && relationshipLinks.some(([, , t]) => t === entity)) {
					entity.cycleState = null;
				}
			});
		});

		this.layers.forEach((layer) => {
			layer.forEach((entity) => {
				entity.update({
					selected: this._selectedEntity === entity,
					mouse: { down: this._mouseDown, delta: this._mouseDelta, position: this._mousePos?.clone() || null } as MouseData
				});
			});
		});

		this.context.clearRect(0, 0, this.canvas.width, this.canvas.height);
		this.context.fillStyle = 'white';
		this.context.fillRect(0, 0, this.canvas.width, this.canvas.height);
		this.context.fillStyle = 'black';
		this.layers.forEach((layer) => {
			layer.forEach((entity) => {
				entity.render(this.renderEngine, {
					selected: this._selectedEntity === entity,
					mouse: null
				});
			});
		});

		if (this._mouseDelta) {
			this._mouseDelta = new Point();
		}
	}

	private _updateSelectedEntity(): void {
		if (this._mousePos) {
			const reversedLayers = this.layers.reduce<Entity[][]>((arr, layer) => [layer, ...arr], []);

			for (const layer of reversedLayers) {
				const reversedEntities = layer.reduce<Entity[]>((arr, entity) => [entity, ...arr], []);

				for (const entity of reversedEntities) {
					if (entity.selectedBy(this._mousePos, this.renderEngine)) {
						this._selectedEntity = entity;
						return;
					}
				}
			}
		}

		this._selectedEntity = null;
	}
}

