import { Module } from '@nestjs/common';
import { DBModule } from 'src/db/db.module';
import { UserService } from './user.service';

@Module({
	imports: [DBModule],
	providers: [UserService],
	exports: [UserService]
})
export class UserModule {}

