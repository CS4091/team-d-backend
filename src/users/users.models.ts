import { Prisma } from '@prisma/client';

export const publicUser = Prisma.validator<Prisma.UserDefaultArgs>()({
	select: {
		id: true,
		name: true,
		email: true
	}
});

export type PublicUser = Prisma.UserGetPayload<typeof publicUser>;

export const meUser = Prisma.validator<Prisma.UserDefaultArgs>()({
	omit: {
		password: true
	},
	include: {
		activeInvites: { include: { organization: { select: { id: true, name: true } } } },
		organizations: true
	}
});

export type MeUser = Prisma.UserGetPayload<typeof meUser>;

