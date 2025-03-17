import { Body, Controller, NotFoundException, Param, Post } from '@nestjs/common';
import { ApiResponse } from '@nestjs/swagger';
import type { Plane, User } from '@prisma/client';
import { Protected } from 'src/auth/protected.decorator';
import { OrganizationIDDTO } from 'src/org/org.dtos';
import { fullOrg } from 'src/org/org.models';
import { OrgService } from 'src/org/org.service';
import { ReqUser } from 'src/utils/decorators/user.decorator';
import { CreatePlaneDTO, PlaneResponse } from './asset.dtos';
import { AssetService } from './asset.service';

@Controller('/organizations/:id/assets')
export class AssetController {
	public constructor(private readonly service: AssetService, private readonly organizations: OrgService) {}

	@Post()
	@Protected()
	@ApiResponse({ type: PlaneResponse })
	public async addPlane(@ReqUser() user: User, @Param() { id }: OrganizationIDDTO, @Body() data: CreatePlaneDTO): Promise<Plane> {
		const org = await this.organizations.get({ id }, fullOrg);

		if (!org || !org.users.some((u) => u.id === user.id)) throw new NotFoundException(`Organization with id '${id}' does not exist.`);

		return this.service.create(data, org);
	}
}

