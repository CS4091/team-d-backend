import { Injectable } from '@nestjs/common';
import { createId } from '@paralleldrive/cuid2';
import type { Organization, Plane } from '@prisma/client';
import { DBService } from 'src/db/db.service';
import { CreatePlaneDTO } from './assets.dtos';
import { fullOrg, FullOrganization } from 'src/orgs/orgs.models';

@Injectable()
export class AssetsService {
	public constructor(private readonly db: DBService) {}

	public async create(data: CreatePlaneDTO, organization: Organization): Promise<Plane> {
		return this.db.plane.create({ data: { id: createId(), ...data, organization: { connect: { id: organization.id } } } });
	}

     //public async deleteAsset(data: )

     public async update(to: FullOrganization, {manufacturer, model, homeBase}: UpdatePlaneDTO): Promise<Plane>{
          return this.db.plane.update({
               where: {id: to.planeID},
               data: {
                    manufacturer,
                    model,
                    homeBase
               },
               ...fullOrg
          });
     }
}

