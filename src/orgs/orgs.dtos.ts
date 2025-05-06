import { ApiProperty } from '@nestjs/swagger';
import type { Organization, Plane, Routing } from '@prisma/client';
import { IsString } from 'class-validator';
import { PlaneResponse } from 'src/assets/assets.dtos';
import { RouteResponse } from 'src/aviation/aviation.dtos';
import { PublicUserResponse } from 'src/users/dtos/public.user.dto';
import { PublicUser } from 'src/users/users.models';
import { fi } from 'src/utils/utils';
import { FullOrganization, PublicInvite } from './orgs.models';

export class CreateOrganizationDTO {
	@IsString()
	@ApiProperty()
	name: string = fi();
}

export class CreateInviteDTO {
	@IsString({ each: true })
	@ApiProperty()
	userIds: string[] = fi();
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

export class OrganizationInviteDTO extends OrganizationIDDTO {
	@IsString()
	@ApiProperty()
	token: string = fi();
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

export class RoutingResponse implements Routing {
	@ApiProperty()
	id: string = fi();

	@ApiProperty()
	orgId: string = fi();

	@ApiProperty({ type: () => RouteResponse })
	data: any = fi();

	@ApiProperty({ type: 'string' })
	createdAt: Date = fi();
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

	@ApiProperty({ type: () => RoutingResponse, isArray: true })
	routings: RoutingResponse[] = fi();
}

