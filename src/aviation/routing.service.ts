import { Injectable } from '@nestjs/common';
import { createId } from '@paralleldrive/cuid2';
import { Plane } from '@prisma/client';
import { ChildProcessWithoutNullStreams, spawn } from 'child_process';
import { mkdirSync, readFileSync, rmSync, statSync, writeFileSync } from 'fs';
import { RouteResult } from './aviation.dtos';
import { Airport, PlaneModel } from './aviation.models';

const OP_ROUTE = new Uint8Array([0]);

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

	public async route(airports: Airport[], routes: [string, string][], assets: (Plane & { specs: PlaneModel })[]): Promise<RouteResult> {
		const opid = createId();
		mkdirSync(`processing/${opid}`);

		let graphSpec = 'graph';
		airports.forEach((airport) => (graphSpec += `\nnode ${airport.name} ${airport.city} ${airport.lat} ${airport.lng}`));
		routes.forEach(([from, to]) => (graphSpec += `\nlink ${from} ${to} 1`));

		writeFileSync(`processing/${opid}/demand.graph`, graphSpec);

		let assetsSpec = '';
		assets.forEach((plane) => (assetsSpec += `${plane.id} ${plane.homeBase} ${plane.specs.landingRunway} ${plane.specs.landingRunway}`));

		writeFileSync(`processing/${opid}/planes.assets`, assetsSpec);

		return this.process(opid).then(() => {
			const plan = JSON.parse(readFileSync(`processing/${opid}/routing.json`).toString());

			// rmSync(`processing/${opid}/routing.json`);
			// rmSync(`processing/${opid}/demand.graph`);
			// rmSync(`processing/${opid}/planes.assets`);
			rmSync(`processing/${opid}`, { recursive: true });

			return plan;
		});
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

			this.proc.stdin.write(OP_ROUTE);
			this.proc.stdin.write(id);

			this.resolvers.set(id, resolve);
		});
	}
}

