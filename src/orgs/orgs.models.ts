import { Prisma } from '@prisma/client';
import { publicUser } from 'src/users/users.models';

export const fullOrg = Prisma.validator<Prisma.OrganizationDefaultArgs>()({
	include: {
		activeInvites: true,
		users: { ...publicUser },
		planes: true
	}
});

export type FullOrganization = Prisma.OrganizationGetPayload<typeof fullOrg>;

