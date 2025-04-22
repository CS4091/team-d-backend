import { ApiProperty } from '@nestjs/swagger';
import type { Organization, Plane } from '@prisma/client';
import { IsString } from 'class-validator';
import { PlaneResponse } from 'src/assets/assets.dtos';
import { PublicUserResponse } from 'src/users/users.dtos';
import { PublicUser } from 'src/users/users.models';
import { fi } from 'src/utils/utils';
import { FullOrganization, PublicInvite } from './orgs.models';

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

export class OrganizationPlaneIDDTO {
	@IsString()
	@ApiProperty()
	id: string = fi();

	@IsString()
	@ApiProperty()
	planeId: string = fi();
}

export class NameUpdateDTO {
	@IsString()
	@ApiProperty()
	name: string = fi();
}

export class OrganizationResponse implements Organization {
	@ApiProperty()
	name: string = fi();

	@ApiProperty()
	id: string = fi();
}

export class InviteResponse implements PublicInvite {
	@ApiProperty()
	userId: string = fi();

	@ApiProperty()
	token: string = fi();

	@ApiProperty()
	orgId: string = fi();

	@ApiProperty()
	createdAt: Date = fi();

	@ApiProperty({ type: () => OrganizationResponse })
	organization: OrganizationResponse = fi();

	@ApiProperty({ type: () => PublicUserResponse })
	user: PublicUserResponse = fi();
}

export class FullOrganizationResponse implements FullOrganization {
	@ApiProperty({ type: () => InviteResponse, isArray: true })
	activeInvites: InviteResponse[] = fi();

	@ApiProperty({ type: () => PublicUserResponse, isArray: true })
	users: PublicUser[] = fi();

	@ApiProperty({ type: () => PlaneResponse, isArray: true })
	planes: Plane[] = fi();

	@ApiProperty()
	name: string = fi();

	@ApiProperty()
	id: string = fi();
}

