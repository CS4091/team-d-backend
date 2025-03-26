import { Injectable } from '@nestjs/common';
import { createId } from '@paralleldrive/cuid2';
import { Plane } from '@prisma/client';
import { ChildProcessWithoutNullStreams, spawn } from 'child_process';
import { mkdirSync, rmSync, statSync, writeFileSync } from 'fs';
import { RouteResult } from './aviation.dtos';
import { Airport, City, PlaneModel } from './aviation.models';

@Injectable()
export class RoutingService {
	private readonly resolvers: Map<string, () => void>;
	private readonly proc: ChildProcessWithoutNullStreams;

	public constructor() {
		try {
			try {
				if (!statSync('processing').isDirectory()) {
					rmSync('processing');
					mkdirSync('processing');
				}
			} catch {
				mkdirSync('processing');
			}
		} catch (err) {
			console.error(err);
			throw new Error(`Unable to acquire 'processing' dir for routing`);
		}

		this.resolvers = new Map();
		this.proc = spawn('./bin/daemon', { cwd: 'processing' });

		let stdout = '';
		this.proc.stdout.on('data', (chunk) => {
			if (typeof chunk === 'string') stdout += chunk;
			else if (Buffer.isBuffer(chunk)) stdout += chunk.toString('utf-8');
			else {
				console.error(chunk);
				throw new Error('Unrecognized chunk type');
			}

			let match = /^(\w{24})\n(.*)$/.exec(stdout);
			while (match) {
				const [, id, rest] = match;

				if (this.resolvers.has(id)) {
					this.resolvers.get(id)!();
					this.resolvers.delete(id);
				}

				stdout = rest;
				match = /^(\w{24})\n(.*)$/.exec(stdout);
			}
		});
	}

	public async route(cities: City[], airports: Airport[], routes: [string, string][], assets: (Plane & { specs: PlaneModel })[]): Promise<RouteResult> {
		const opid = createId();
		mkdirSync(`processing/${opid}`);

		writeFileSync(`processing/${opid}/cities.json`, JSON.stringify(cities));
		writeFileSync(`processing/${opid}/airports.json`, JSON.stringify(airports));
		writeFileSync(`processing/${opid}/routes.json`, JSON.stringify(routes.map(([from, to]) => ({ from, to }))));
		writeFileSync(`processing/${opid}/planes.json`, JSON.stringify(assets.map(({ id, homeBase, specs }) => ({ id, homeBase, ...specs }))));

		// return this.process(opid).then(() => {
		// 	const plan = JSON.parse(readFileSync(`processing/${opid}/routing.json`).toString());

		// 	// rmSync(`processing/${opid}/routing.json`);
		// 	// rmSync(`processing/${opid}/demand.graph`);
		// 	// rmSync(`processing/${opid}/planes.assets`);
		// 	rmSync(`processing/${opid}`, { recursive: true });

		// 	return plan;
		// });
		return {} as any;
	}

	public async process(id: string): Promise<void> {
		return new Promise<void>((resolve, reject) => {
			try {
				if (!statSync(`processing/${id}`).isDirectory()) {
					reject(new Error(`Path 'processing' is not directory`));
				}
			} catch {
				reject(new Error(`Unable to acquire 'processing' dir for routing`));
			}

			this.proc.stdin.write(id + '\n');

			this.resolvers.set(id, resolve);
		});
	}
}

