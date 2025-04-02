import { Controller, Get, Header, Query } from '@nestjs/common';
import { readFileSync } from 'fs';
import { Page } from './utils/decorators/page.decorator';

@Controller()
export class AppController {
	constructor() {}

	@Get('/')
	@Page()
	public index() {}

	@Get('/demo-graph')
	public getDemoGraph(): any {
		return {
			connectivity: JSON.parse(readFileSync(`test.graph.c.json`).toString()),
			demand: JSON.parse(readFileSync(`test.graph.d.json`).toString()),
			path: JSON.parse(readFileSync(`test.graph.p.json`).toString()),
			baseline: JSON.parse(readFileSync(`test.graph.b.json`).toString())
		};
	}

	@Get('/regraph')
	@Header('Content-Type', 'application/json')
	public async regraph(@Query('workspace') workspace: string): Promise<any> {
		return {
			connectivity: JSON.parse(readFileSync(`processing/${workspace}/routing.graph.c.json`).toString()),
			// demand: JSON.parse(readFileSync(`processing/${workspace}/routing.graph.d.json`).toString()),
			// path: JSON.parse(readFileSync(`processing/${workspace}/routing.graph.p.json`).toString()),
			baseline: JSON.parse(readFileSync(`processing/${workspace}/routing.graph.b.json`).toString())
		};
	}
}

