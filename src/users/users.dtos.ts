import { ApiProperty } from '@nestjs/swagger';
import type { Organization } from '@prisma/client';
import { IsEmail, IsString } from 'class-validator';
import { InviteResponse, OrganizationResponse } from 'src/orgs/orgs.dtos';
import { PublicInvite } from 'src/orgs/orgs.models';
import { fi } from 'src/utils/utils';
import { MeUser } from './users.models';

export class RegisterDTO {
	@IsString()
	@ApiProperty()
	name: string = fi();

	@IsEmail()
	@ApiProperty()
	email: string = fi();

	@IsString()
	// @IsStrongPassword()
	@ApiProperty()
	password: string = fi();
}

export class LoginDTO {
	@IsEmail()
	@ApiProperty()
	email: string = fi();

	@IsString()
	// @IsStrongPassword()
	@ApiProperty()
	password: string = fi();
}

export class UpdateNameDTO {
	@IsString()
	@ApiProperty()
	name: string = fi();
}

export class MeUserResponse implements MeUser {
	@ApiProperty()
	name: string = fi();

	@ApiProperty()
	email: string = fi();

	@ApiProperty()
	id: string = fi();

	@ApiProperty()
	token: string = fi();

	@ApiProperty({ type: () => OrganizationResponse, isArray: true })
	organizations: Organization[] = fi();

	@ApiProperty({ type: () => InviteResponse, isArray: true })
	activeInvites: PublicInvite[] = fi();
}

