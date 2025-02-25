import { Controller, Get, Post, StreamableFile, UploadedFile, UseInterceptors } from '@nestjs/common';
import { FileInterceptor } from '@nestjs/platform-express';
import { spawn } from 'child_process';
import { randomBytes } from 'crypto';
import { createReadStream, readFileSync, rmSync, writeFileSync } from 'fs';
import { AppService } from './app.service';
import { Page } from './utils/decorators/page.decorator';

@Controller()
export class AppController {
	constructor(private readonly appService: AppService) {}

	@Get('/')
	@Page()
	public index() {}

	@Get('/demo-graph')
	public getDemoGraph(): StreamableFile {
		return new StreamableFile(createReadStream('test.graph.json'));
	}

	@Post('/regraph')
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
					resolve(readFileSync(`${id}.graph.json`).toString());
				} else {
					console.error('Error child', status);
					console.error(stdout);
					console.error(stderr);

					reject(stderr);
				}

				rmSync(`${id}.graph`);
				rmSync(`${id}.graph.json`);
			});
		});
	}
}

