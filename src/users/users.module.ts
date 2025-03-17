import { Module } from '@nestjs/common';
import { DBModule } from 'src/db/db.module';
import { UsersService } from './users.service';

@Module({
	imports: [DBModule],
	providers: [UsersService],
	exports: [UsersService]
})
export class UsersModule {}

