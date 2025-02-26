<script lang="ts">
	import { Engine } from '$lib/engine/engine';

	let canvas: HTMLCanvasElement = $state(null),
		files: FileList = $state(null),
		form: HTMLFormElement,
		engine: Engine;

	$effect(() => {
		if (canvas) {
			engine = new Engine(canvas);

			(window as any).engine = engine;

			engine.start();

			fetch('/demo-graph')
				.then((res) => res.json())
				.then((data) => engine.load(data));
		}
	});

	function reload() {
		if (files) {
			const fd = new FormData();
			fd.append('file', files[0]);

			fetch('/regraph', { method: 'POST', body: fd })
				.then((res) => res.json())
				.then((data) => {
					engine.load(data);
					form.reset();
				});
		}
	}
</script>

<form bind:this={form}>
	<input type="file" bind:files />
</form>
<button onclick={reload}>Upload</button>
<canvas width={1200} height={800} bind:this={canvas}></canvas>
