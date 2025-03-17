import { ApiProperty } from '@nestjs/swagger';
import type { Invite, Organization } from '@prisma/client';
import { IsEmail, IsString, IsStrongPassword } from 'class-validator';
import { InviteResponse, OrganizationResponse } from 'src/org/org.dtos';
import { fi } from 'src/utils/utils';
import { MeUser, PublicUser } from './user.models';

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
	@IsStrongPassword()
	password: string = fi();
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

