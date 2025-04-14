import { ApiProperty } from '@nestjs/swagger';
import type { Invite, Organization } from '@prisma/client';
import { IsEmail, IsString } from 'class-validator';
import { InviteResponse, OrganizationResponse } from 'src/orgs/orgs.dtos';
import { fi } from 'src/utils/utils';
import { MeUser, PublicUser } from './users.models';

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

export class PublicUserResponse implements PublicUser {
	@ApiProperty()
	name: string = fi();

	@ApiProperty()
	id: string = fi();
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
	activeInvites: Invite[] = fi();
}

