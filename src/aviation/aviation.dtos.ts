import { ApiProperty } from '@nestjs/swagger';
import { Type } from 'class-transformer';
import { IsArray, IsInt, IsString, ValidateNested } from 'class-validator';
import { fi } from 'src/utils/utils';

export class RouteRequest {
	@IsString()
	@ApiProperty()
	from: string = fi();

	@IsString()
	@ApiProperty()
	to: string = fi();

	@IsInt()
	@ApiProperty()
	passengers: number = fi();
}

export class RouteScenario {
	@IsString()
	@ApiProperty()
	organizationId: string = fi();

	@IsArray()
	@Type(() => RouteRequest)
	@ValidateNested({ each: true })
	@ApiProperty({ type: RouteRequest, isArray: true })
	demand: RouteRequest[] = fi();
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

export class RouteResponse {
	@ApiProperty({ type: RouteResult })
	baseline: RouteResult = fi();

	@ApiProperty({ type: RouteResult })
	optimized: RouteResult = fi();
}

