<script lang="ts">
	import { Engine } from '$lib/engine/engine';
	import type { Entity } from '$lib/engine/entity';

	let canvas: HTMLCanvasElement = $state(null),
		files: FileList = $state(null),
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
					edges.connectivity = engine.loadConnectivity(data.connectivity);
					edges.demand = engine.loadDemand(data.demand);
					edges.path = engine.loadPath(data.path);

					engine.load(edges.connectivity);
				});
		}
	});

	function reload() {
		if (files) {
			const fd = new FormData();
			fd.append('file', files[0]);

			fetch('/regraph', { method: 'POST', body: fd })
				.then((res) => res.json())
				.then((data) => {
					edges.connectivity = engine.loadConnectivity(data.connectivity);
					edges.demand = engine.loadDemand(data.demand);
					edges.path = engine.loadPath(data.path);

					engine.load(edges.connectivity);

					form.reset();
				});
		}
	}
</script>

<form bind:this={form}>
	<input type="file" bind:files />
</form>
<div class="row">
	<button onclick={reload}>Upload</button>
	<button onclick={() => engine.load(edges.connectivity)}>Connectivity</button>
	<button onclick={() => engine.load(edges.demand)}>Demand</button>
	<button onclick={() => engine.load(edges.path)}>Path</button>
</div>
<canvas width={1200} height={800} bind:this={canvas}></canvas>
