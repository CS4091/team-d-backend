import { Injectable, NotFoundException } from '@nestjs/common';
import { createId } from '@paralleldrive/cuid2';
import type { Prisma, User } from '@prisma/client';
import { PrismaClientKnownRequestError } from '@prisma/client/runtime/library';
import { compareSync, genSaltSync, hashSync } from 'bcrypt';
import { randomBytes } from 'crypto';
import { AuthDataSource } from 'src/auth/auth.module';
import { DBService } from 'src/db/db.service';
import { wrap } from 'src/utils/utils';
import { LoginDTO, RegisterDTO } from './user.dtos';
import { MeUser, meUser } from './user.models';

@Injectable()
export class UserService implements AuthDataSource {
	public constructor(public readonly db: DBService) {}

	public async get<S extends Prisma.UserDefaultArgs>(
		where: Prisma.UserWhereUniqueInput,
		selectors: S = {} as any
	): Promise<Prisma.UserGetPayload<S> | null> {
		return this.db.user.findUnique({ where, ...selectors }) as any;
	}

	public async login({ email, password }: LoginDTO): Promise<MeUser | null> {
		const user = await this.db.user.findUnique({ where: { email } });

		// avoid leaking existence data (via timing)
		return !compareSync(password, user?.password ?? '') ? null : this.db.user.findUnique({ where: { email }, ...meUser });
	}

	public async register({ name, email, password }: RegisterDTO): Promise<MeUser> {
		const id = createId();
		const hashedPassword = hashSync(password, genSaltSync());

		return this.db.user
			.create({
				data: {
					id,
					name,
					email,
					password: hashedPassword,
					token: randomBytes(32).toString('hex')
				},
				...meUser
			})
			.catch((err) => {
				if (err instanceof PrismaClientKnownRequestError && err.code === 'P2002') {
					return this.register({ name, email, password });
				} else {
					throw err;
				}
			});
	}

	public async auth(token: string): Promise<User | null> {
		return this.db.user.findUnique({ where: { token } });
	}

	public static readonly NotFoundException = class extends wrap(NotFoundException) {};
}

