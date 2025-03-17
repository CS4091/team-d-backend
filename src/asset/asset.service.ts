import { Injectable } from '@nestjs/common';
import { createId } from '@paralleldrive/cuid2';
import type { Organization, Plane } from '@prisma/client';
import { DBService } from 'src/db/db.service';
import { CreatePlaneDTO } from './asset.dtos';

@Injectable()
export class AssetService {
	public constructor(private readonly db: DBService) {}

	public async create(data: CreatePlaneDTO, organization: Organization): Promise<Plane> {
		return this.db.plane.create({ data: { id: createId(), ...data, organization: { connect: { id: organization.id } } } });
	}
}

