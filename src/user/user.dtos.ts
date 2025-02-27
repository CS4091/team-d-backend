import { IsEmail, IsString } from 'class-validator';
import { fi } from 'src/utils/utils';

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
