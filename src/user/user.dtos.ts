import { ApiProperty } from '@nestjs/swagger';
import { IsEmail, IsString } from 'class-validator';
import { fi } from 'src/utils/utils';
import { PublicUser } from './user.models';

export class RegisterDTO {
	@IsString()
	name: string = fi();

	@IsEmail()
	email: string = fi();

	@IsString()
	password: string = fi();
}

export class LoginDTO {
	@IsEmail()
	email: string = fi();

	@IsString()
	password: string = fi();
}

export class PublicUserResponse implements PublicUser {
	@ApiProperty()
	name: string = fi();

	@ApiProperty()
	id: string = fi();
}
