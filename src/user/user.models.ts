import { Prisma } from '@prisma/client';

export const publicUser = Prisma.validator<Prisma.UserDefaultArgs>()({
	select: {
		id: true,
		name: true
	}
});

export type PublicUser = Prisma.UserGetPayload<typeof publicUser>;

export const meUser = Prisma.validator<Prisma.UserDefaultArgs>()({
	omit: {
		password: true
	},
	include: {
		activeInvites: true,
		organizations: true
	}
});

export type MeUser = Prisma.UserGetPayload<typeof meUser>;

