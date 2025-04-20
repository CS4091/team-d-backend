import { Injectable } from '@nestjs/common';
import { createId } from '@paralleldrive/cuid2';
import type { Organization, Plane } from '@prisma/client';
import { DBService } from 'src/db/db.service';
import { CreatePlaneDTO, UpdatePlaneDTO } from './assets.dtos';

@Injectable()
export class AssetsService {
	public constructor(private readonly db: DBService) {}

	public async create(data: CreatePlaneDTO, organization: Organization): Promise<Plane> {
		return this.db.plane.create({ data: { id: createId(), ...data, organization: { connect: { id: organization.id } } } });
	}

	public async delete(organization: Organization, id: string) {
                return this.db.plane.delete({
                    where: { id_orgId: { id, orgId: organization.id } },
                });
    }

	public async update(data: UpdatePlaneDTO, organization: Organization, id: string): Promise<Plane> {
		return this.db.plane.update({
			where: { id_orgId: { id, orgId: organization.id } },
			data
		});
	}
}

