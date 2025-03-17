import { ApiProperty } from '@nestjs/swagger';
import type { Invite, Organization, Plane } from '@prisma/client';
import { IsString } from 'class-validator';
import { PlaneResponse } from 'src/assets/assets.dtos';
import { PublicUserResponse } from 'src/users/users.dtos';
import { PublicUser } from 'src/users/users.models';
import { fi } from 'src/utils/utils';
import { FullOrganization } from './orgs.models';

export class CreateOrganizationDTO {
	@IsString()
	@ApiProperty()
	name: string = fi();
}

export class CreateInviteDTO {
	@IsString()
	@ApiProperty()
	userId: string = fi();
}

export class AcceptInviteDTO {
	@IsString()
	@ApiProperty()
	token: string = fi();
}

export class OrganizationIDDTO {
	@IsString()
	@ApiProperty()
	id: string = fi();
}

export class OrganizationResponse implements Organization {
	@ApiProperty()
	name: string = fi();

	@ApiProperty()
	id: string = fi();
}

export class InviteResponse implements Invite {
	@ApiProperty()
	userId: string = fi();

	@ApiProperty()
	token: string = fi();

	@ApiProperty()
	orgId: string = fi();

	@ApiProperty()
	createdAt: Date = fi();
}

export class FullOrganizationResponse implements FullOrganization {
	@ApiProperty({ type: () => InviteResponse, isArray: true })
	activeInvites: Invite[] = fi();

	@ApiProperty({ type: () => PublicUserResponse, isArray: true })
	users: PublicUser[] = fi();

	@ApiProperty({ type: () => PlaneResponse, isArray: true })
	planes: Plane[] = fi();

	@ApiProperty()
	name: string = fi();

	@ApiProperty()
	id: string = fi();
}

