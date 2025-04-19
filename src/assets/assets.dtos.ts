import { ApiProperty } from '@nestjs/swagger';
import type { Plane } from '@prisma/client';
import { IsString } from 'class-validator';
import { fi } from 'src/utils/utils';

export class CreatePlaneDTO {
	@IsString()
	@ApiProperty()
	manufacturer: string = fi();

	@IsString()
	@ApiProperty()
	model: string = fi();

	@IsString()
	@ApiProperty()
	homeBase: string = fi();
}

export class UpdatePlaneDTO {
	@IsString()
	@ApiProperty()
	manufacturer: string = fi();

	@IsString()
	@ApiProperty()
	model: string = fi();

	@IsString()
	@ApiProperty()
	homeBase: string = fi();
}

export class PlaneResponse implements Plane {
	@ApiProperty()
	id: string = fi();

	@ApiProperty()
	orgId: string = fi();

	@ApiProperty()
	manufacturer: string = fi();

	@ApiProperty()
	model: string = fi();

	@ApiProperty()
	homeBase: string = fi();
}

export class PlaneIDDTO {
     @IsString()
     @ApiProperty()
     id: string = fi();
}


