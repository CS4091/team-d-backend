import { ApiProperty } from '@nestjs/swagger';
import { fi } from 'src/utils/utils';
import { PublicUser } from '../users.models';

export class PublicUserResponse implements PublicUser {
	@ApiProperty()
	name: string = fi();

	@ApiProperty()
	id: string = fi();

	@ApiProperty()
	email: string = fi();
}
