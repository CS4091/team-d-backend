<script lang="ts">
	import { Engine } from '$lib/engine/engine';
	import type { Entity } from '$lib/engine/entity';

	let workspace: string = $state(''),
		canvas: HTMLCanvasElement = $state(null),
		form: HTMLFormElement,
		engine: Engine,
		edges: Record<string, Entity[]> = {};

	$effect(() => {
		if (canvas) {
			engine = new Engine(canvas);

			(window as any).engine = engine;

			engine.start();

			fetch('/demo-graph')
				.then((res) => res.json())
				.then((data) => {
					edges.connectivity = engine.loadGraph(data.connectivity);
					edges.demand = engine.loadGraph(data.demand, 'blue');
					edges.path = engine.loadGraph(data.path, 'red');
					edges.baseline = engine.loadGraph(data.baseline, 'red');

					engine.load(edges.connectivity);
				});
		}
	});

	function reload() {
		fetch(`/regraph?workspace=${workspace}`)
			.then((res) => res.json())
			.then((data) => {
				(engine as any).layers = [[], []];

				edges.connectivity = engine.loadGraph(data.connectivity);
				// edges.demand = engine.loadGraph(data.demand);
				// edges.path = engine.loadGraph(data.path);
				edges.baseline = engine.loadGraph(data.baseline);

				engine.load(edges.connectivity);

				form.reset();
			});
	}
</script>

<form bind:this={form}>
	<input type="text" bind:value={workspace} />
</form>
<div class="row">
	<button onclick={reload}>Upload</button>
	<button onclick={() => engine.load(edges.connectivity)}>Connectivity</button>
	<button onclick={() => engine.load(edges.demand)}>Demand</button>
	<button onclick={() => engine.load(edges.path)}>Path</button>
	<button onclick={() => engine.load(edges.baseline)}>Baseline</button>
</div>
<canvas width={1200} height={800} bind:this={canvas}></canvas>
