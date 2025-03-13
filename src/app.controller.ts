import { Controller, Get, Header, Post, UploadedFile, UseInterceptors } from '@nestjs/common';
import { FileInterceptor } from '@nestjs/platform-express';
import { ApiResponse } from '@nestjs/swagger';
import { spawn } from 'child_process';
import { randomBytes } from 'crypto';
import { readFileSync, rmSync, writeFileSync } from 'fs';
import { AppService } from './app.service';
import { Airport } from './aviation/aviation.models';
import { AviationService } from './aviation/aviation.service';
import { Page } from './utils/decorators/page.decorator';

@Controller()
export class AppController {
	constructor(private readonly appService: AppService, public readonly avaiation: AviationService) {}

	@Get('/')
	@Page()
	public index() {}

	@Get('/demo-graph')
	public getDemoGraph(): any {
		return {
			connectivity: JSON.parse(readFileSync(`test.graph.c.json`).toString()),
			demand: JSON.parse(readFileSync(`test.graph.d.json`).toString()),
			path: JSON.parse(readFileSync(`test.graph.p.json`).toString())
		};
	}

	@Get('/airports')
	@ApiResponse({ type: Airport })
	public async getAirports(): Promise<Airport[]> {
		return this.avaiation.airports;
	}

	@Post('/regraph')
	@Header('Content-Type', 'application/json')
	@UseInterceptors(FileInterceptor('file'))
	public async regraph(@UploadedFile() file: Express.Multer.File): Promise<any> {
		const id = randomBytes(16).toString('hex');

		writeFileSync(`${id}.graph`, file.buffer);

		return new Promise((resolve, reject) => {
			let stdout = '',
				stderr = '';

			const proc = spawn('./bin/demo', [`${id}.graph`]);

			proc.stdout.on('data', (c) => (stdout += c.toString()));
			proc.stderr.on('data', (c) => (stderr += c.toString()));

			proc.on('exit', (status) => {
				if (status === 0) {
					resolve({
						connectivity: JSON.parse(readFileSync(`${id}.graph.c.json`).toString()),
						demand: JSON.parse(readFileSync(`${id}.graph.d.json`).toString()),
						path: JSON.parse(readFileSync(`test.graph.p.json`).toString())
					});
				} else {
					console.error('Error child', status);
					console.error(stdout);
					console.error(stderr);

					reject(stderr);
				}

				rmSync(`${id}.graph`);
				rmSync(`${id}.graph.c.json`);
				rmSync(`${id}.graph.d.json`);
				rmSync(`${id}.graph.p.json`);
			});
		});
	}
}

