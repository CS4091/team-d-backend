import { ApiProperty } from '@nestjs/swagger';
import { IsArray, IsString } from 'class-validator';
import { fi } from 'src/utils/utils';

export class RouteScenario {
	@IsString()
	@ApiProperty()
	organizationId: string = fi();

	@IsArray({ each: true })
	@ApiProperty({
		type: 'array',
		items: { type: 'array', maxLength: 2, minLength: 2, example: ['from', 'to'], items: { type: 'string' } },
		description: 'Array of [from, to] tuples indicating desired routes'
	})
	demand: [string, string][] = fi();
}

export class RouteStats {
	@ApiProperty()
	fuel: number = fi();
}

export class RouteResult {
	@ApiProperty({
		example: { 'example-id': ['A', 'B', 'C'] },
		description: 'Record of plane routes, where key is id and value is array of cities traversed (beginning with home base)'
	})
	routing: Record<string, string[]> = fi();

	@ApiProperty({ type: RouteStats })
	stats: RouteStats = fi();
}

