import { BadRequestException, ForbiddenException, Injectable, NotFoundException } from '@nestjs/common';
import { createId } from '@paralleldrive/cuid2';
import type { Invite, Prisma, User } from '@prisma/client';
import { randomBytes } from 'crypto';
import { DateTime } from 'luxon';
import { DBService } from 'src/db/db.service';
import { wrap } from 'src/utils/utils';
import { CreateOrganizationDTO } from './orgs.dtos';
import { fullOrg, FullOrganization } from './orgs.models';

@Injectable()
export class OrgsService {
	public constructor(private readonly db: DBService) {}

	public async get<S extends Prisma.OrganizationDefaultArgs>(
		where: Prisma.OrganizationWhereUniqueInput,
		selectors: S = {} as any
	): Promise<Prisma.OrganizationGetPayload<S>> {
		return this.db.organization.findUnique({ where, ...selectors }) as any;
	}

	public async getOrThrow<S extends Prisma.OrganizationDefaultArgs>(
		where: Prisma.OrganizationWhereUniqueInput,
		selectors: S = {} as any
	): Promise<Prisma.OrganizationGetPayload<S>> {
		const org = await this.db.organization.findUnique({ where, ...selectors });

		if (!org) throw new OrgsService.NotFoundException();

		return org as any;
	}

	public async getAll<S extends Prisma.OrganizationDefaultArgs>(
		where: Prisma.OrganizationWhereInput,
		selectors: S = {} as any
	): Promise<Prisma.OrganizationGetPayload<S>[]> {
		return this.db.organization.findMany({ where, ...selectors }) as any;
	}

	public async create(user: User, data: CreateOrganizationDTO): Promise<FullOrganization> {
		return this.db.organization.create({ data: { id: createId(), ...data, users: { connect: { id: user.id } } }, ...fullOrg });
	}

	public async invite(to: FullOrganization, user: User): Promise<Invite> {
		if (to.users.some((u) => u.id === user.id)) throw new OrgsService.DuplicateException();

		return this.db.invite.create({ data: { orgId: to.id, userId: user.id, token: randomBytes(32).toString('hex') } });
	}

	public async acceptInvite(user: User, token: string): Promise<FullOrganization> {
		const invite = await this.db.invite.findUnique({ where: { token } });

		if (!invite || invite.userId !== user.id) throw new OrgsService.NotAllowedException();
		if (DateTime.fromJSDate(invite.createdAt).diffNow().get('days') > 1) {
			await this.db.invite.delete({ where: { token } }); // realistically should also run cron job or something to periodically cleanup db
			throw new OrgsService.InviteExpiredException();
		}

		return this.db.organization.update({
			where: { id: invite.orgId },
			data: { users: { connect: { id: invite.userId } }, activeInvites: { delete: { token } } },
			...fullOrg
		});
	}

	public async updateOrgName(to: FullOrganization, name: string): Promise<FullOrganization> {
		return this.db.organization.update({
			where: { id: to.id },
			data: { name },
			...fullOrg
		});
	}

	public static readonly NotFoundException = class extends wrap(NotFoundException) {};
	public static readonly NotAllowedException = class extends wrap(ForbiddenException) {};
	public static readonly DuplicateException = class extends wrap(BadRequestException) {};
	public static readonly InviteExpiredException = class extends wrap(BadRequestException) {};
}

