import { Injectable } from '@nestjs/common';
import { createId } from '@paralleldrive/cuid2';
import type { Plane } from '@prisma/client';
import { ChildProcessWithoutNullStreams, spawn } from 'child_process';
import { existsSync, mkdirSync, readFileSync, rmSync, statSync, writeFileSync } from 'fs';
import { cwd } from 'process';
import { RouteResult } from './aviation.dtos';
import { City, PlaneModel } from './aviation.models';

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
		this.proc = spawn(`${cwd()}/bin/daemon`, { cwd: 'processing' });
		this.proc.on('error', (err) => console.error(err));

		this.proc.stdin.on('error', (error: NodeJS.ErrnoException) => {
			if (error.code === 'EPIPE') console.error('EPIPE error: child process pipe is closed.', error);
			else console.error('Error on stdin:', error);
		});

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
		this.proc.stderr.on('data', (chunk) => {
			if (typeof chunk === 'string') console.error(chunk);
			else if (Buffer.isBuffer(chunk)) console.error(chunk.toString('utf-8'));
			else {
				console.error(chunk);
				throw new Error('Unrecognized chunk type');
			}
		});
	}

	public async route(cities: City[], routes: [string, string][], assets: (Plane & { specs: PlaneModel })[]): Promise<RouteResult> {
		const opid = createId();
		mkdirSync(`processing/${opid}`);

		writeFileSync(`processing/${opid}/cities.json`, JSON.stringify(cities));
		writeFileSync(`processing/${opid}/routes.json`, JSON.stringify(routes.map(([from, to]) => ({ from, to }))));
		writeFileSync(`processing/${opid}/planes.json`, JSON.stringify(assets.map(({ id, homeBase, specs }) => ({ id, homeBase, ...specs }))));

		return this.process(opid).then(() => {
			try {
				const plan = JSON.parse(readFileSync(`processing/${opid}/routing.json`).toString());

				rmSync(`processing/${opid}`, { recursive: true });

				return plan;
			} catch {
				const errs = JSON.parse(readFileSync(`processing/${opid}/errors.json`).toString());

				rmSync(`processing/${opid}`, { recursive: true });

				throw errs;
			}
		});
	}

	public async prep(cities: City[], planes: PlaneModel[]): Promise<void> {
		if (!existsSync('processing/test')) {
			mkdirSync('processing/test');

			writeFileSync('processing/test/cities.json', JSON.stringify(cities));
			writeFileSync('processing/test/planes.json', JSON.stringify(planes));
		}
	}

	public async process(id: string): Promise<void> {
		return new Promise<void>((resolve, reject) => {
			try {
				if (!statSync(`processing/${id}`).isDirectory()) {
					reject(new Error("Path 'processing' is not directory"));
				}
			} catch {
				reject(new Error("Unable to acquire 'processing' dir for routing"));
			}

			this.proc.stdin.write(id + '\n');

			this.resolvers.set(id, resolve);
		});
	}
}

