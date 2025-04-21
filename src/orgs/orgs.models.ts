import { Prisma } from '@prisma/client';
import { publicUser } from 'src/users/users.models';

export const publicInvite = Prisma.validator<Prisma.InviteDefaultArgs>()({
	include: {
		organization: true
	}
});

export type PublicInvite = Prisma.InviteGetPayload<typeof publicInvite>;

export const fullOrg = Prisma.validator<Prisma.OrganizationDefaultArgs>()({
	include: {
		activeInvites: publicInvite,
		users: publicUser,
		planes: true
	}
});

export type FullOrganization = Prisma.OrganizationGetPayload<typeof fullOrg>;

