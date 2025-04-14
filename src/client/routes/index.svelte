<script lang="ts">
	import { Engine } from '$lib/engine/engine';
	import type { Entity } from '$lib/engine/entity';
	import { SvelteMap } from 'svelte/reactivity';

	let files: FileList = $state(null),
		canvas: HTMLCanvasElement = $state(null),
		form: HTMLFormElement,
		engine: Engine,
		graphs: Map<string, Entity[]> = $state(new SvelteMap());

	$effect(() => {
		if (canvas) {
			engine = new Engine(canvas);

			(window as any).engine = engine;

			engine.start();
		}
	});

	function loadNodes(): void {
		files[0].text().then((json) => {
			const graph = JSON.parse(json);

			engine.loadNodes(graph);
			form.reset();
		});
	}

	function loadEdges(): void {
		files[0].text().then((json) => {
			const graph = JSON.parse(json);

			graphs.set(files[0].name, engine.loadEdges(graph));
			form.reset();
		});
	}
</script>

<form bind:this={form}>
	<input type="file" bind:files />
</form>
<div class="row">
	<button onclick={loadNodes}>Load Nodes</button>
	<button onclick={loadEdges}>Load Edges</button>
	{#each graphs as [file, graph]}
		<button onclick={() => engine.load(graph)}>{file}</button>
	{/each}
</div>
<canvas width={1200} height={800} bind:this={canvas}></canvas>
